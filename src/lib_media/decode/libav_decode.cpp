#include "libav_decode.hpp"
#include "../common/pcm.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "lib_ffpp/ffpp.hpp"
#include <cassert>

namespace {
auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);
}

namespace Decode {

LibavDecode::LibavDecode(const MetadataPktLibav &metadata)
	: codecCtx(avcodec_alloc_context3(nullptr)), avFrame(new ffpp::Frame), m_numFrames(0) {
	avcodec_copy_context(codecCtx, metadata.getAVCodecContext());

	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
	case AVMEDIA_TYPE_AUDIO:
		break;
	default:
		Log::msg(Log::Warning, "Module LibavDecode: codec_type %s not supported. Must be audio or video.", codecCtx->codec_type);
		throw std::runtime_error("[LibavDecode] Unknown decode type. Failed.");
	}

	//find an appropriate decode
	auto codec = avcodec_find_decoder(codecCtx->codec_id);
	if (!codec) {
		Log::msg(Log::Warning, "Module LibavDecode: Codec not found");
		throw std::runtime_error("[LibavDecode] Decoder not found.");
	}

	//force single threaded as h264 probing seems to miss SPS/PPS and seek fails silently
	ffpp::Dict dict;
	dict.set("threads", "1");

	if (avcodec_open2(codecCtx, codec, &dict) < 0) {
		Log::msg(Log::Warning, "Module LibavDecode: Couldn't open stream");
		throw std::runtime_error("[LibavDecode] Couldn't open stream.");
	}

	auto Metadata_new = new MetadataPktLibav(codecCtx);
	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO: videoPin = addOutputPin(new PinPicture(Metadata_new)); break;
	case AVMEDIA_TYPE_AUDIO: audioPin = addOutputPin(new PinPcm(Metadata_new)); break;
	default: throw std::runtime_error("[LibavDecode] Invalid Pin type.");
	}
}

LibavDecode::~LibavDecode() {
	avcodec_close(codecCtx);
	av_free(codecCtx);
}

bool LibavDecode::processAudio(const DataAVPacket *data) {
	AVPacket *pkt = data->getPacket();
	int gotFrame;
	if (avcodec_decode_audio4(codecCtx, avFrame->get(), &gotFrame, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding audio.");
		return false;
	}
	if (gotFrame) {
		auto out = audioPin->getBuffer(0);
		PcmFormat pcmFormat;
		libavFrame2pcmConvert(avFrame->get(), &pcmFormat);
		out->setFormat(pcmFormat);
		for (uint8_t i = 0; i < pcmFormat.numPlanes; ++i) {
			out->setPlane(i, avFrame->get()->data[i], avFrame->get()->linesize[0] / pcmFormat.numPlanes);
		}
		setTimestamp(out, avFrame->get()->nb_samples);
		audioPin->emit(out);
		++m_numFrames;
		return true;
	}

	return false;
}

namespace {
//FIXME: this function is related to Picture and libav and should not be in a module (libav.xpp) + we can certainly avoid a memcpy here
void copyToPicture(AVFrame const* avFrame, Picture* pic) {
	for (size_t comp=0; comp<pic->getNumPlanes(); ++comp) {
		auto subsampling = comp == 0 ? 1 : 2;
		auto src = avFrame->data[comp];
		auto srcPitch = avFrame->linesize[comp];

		auto dst = pic->getPlane(comp);
		auto dstPitch = pic->getPitch(comp);

		auto const h = avFrame->height / subsampling;
		//auto const w = avFrame->width / subsampling;

		for (int y=0; y<h; ++y) {
			memcpy(dst, src, srcPitch);
			src += srcPitch;
			dst += dstPitch;
		}
	}
}
}

bool LibavDecode::processVideo(const DataAVPacket *data) {
	AVPacket *pkt = data->getPacket();
	int gotPicture;
	if (avcodec_decode_video2(codecCtx, avFrame->get(), &gotPicture, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding video.");
		return false;
	}
	if (gotPicture) {
		auto pic = Picture::create(videoPin, Resolution(avFrame->get()->width, avFrame->get()->height), libavPixFmt2PixelFormat((AVPixelFormat)avFrame->get()->format));
		copyToPicture(avFrame->get(), pic.get());
		setTimestamp(pic);
		videoPin->emit(pic);
		++m_numFrames;
		return true;
	}

	return false;
}

void LibavDecode::setTimestamp(std::shared_ptr<Data> s, uint64_t increment) const {
	uint64_t t;
	if (m_numFrames == 0) {
		t = 0;
	} else if (codecCtx->time_base.den == 0) {
		throw std::runtime_error("[LibavDecode] Unknown frame rate. Cannot set the timestamp.");
	} else {
		auto const curTime = m_numFrames * increment * codecCtx->time_base.num * codecCtx->ticks_per_frame;
		t = timescaleToClock(curTime, codecCtx->time_base.den);
	}
	s->setTime(t);
}

void LibavDecode::process(std::shared_ptr<const Data> data) {
	auto decoderData = safe_cast<const DataAVPacket>(data);
	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		processVideo(decoderData.get());
		break;
	case AVMEDIA_TYPE_AUDIO:
		processAudio(decoderData.get());
		break;
	default:
		assert(0);
		return;
	}
}

void LibavDecode::flush() {
	auto nullPkt = uptr(new DataAVPacket(0));
	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		while (processVideo(nullPkt.get())) {}
		break;
	case AVMEDIA_TYPE_AUDIO:
		while (processAudio(nullPkt.get())) {}
		break;
	default:
		assert(0);
		break;
	}
}

}
