#include "libav_decode.hpp"
#include "internal/core/clock.hpp"
#include "../common/pcm.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include <cassert>

#include "lib_ffpp/ffpp.hpp"

namespace {
auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);
}

namespace Decode {

LibavDecode::LibavDecode(const PropsDecoder &props)
	: codecCtx(avcodec_alloc_context3(NULL)), avFrame(new ffpp::Frame), m_numFrames(0) {
	avcodec_copy_context(codecCtx, props.getAVCodecContext());

	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
	case AVMEDIA_TYPE_AUDIO:
		break;
	default:
		Log::msg(Log::Warning, "Module LibavDecode: codec_type %s not supported. Must be audio or video.", codecCtx->codec_type);
		throw std::runtime_error("[LibavDecode] Unknown decoder type. Failed.");
	}

	//find an appropriate decoder
	auto codec = avcodec_find_decoder(codecCtx->codec_id);
	if (!codec) {
		Log::msg(Log::Warning, "Module LibavDecode: Codec not found");
		throw std::runtime_error("[LibavDecode] Decoder not found.");
	}

	//TODO: test: force single threaded as h264 probing seems to miss SPS/PPS and seek fails silently
	ffpp::Dict dict;
	dict.set("threads", "1");

	//open the codec
	if (avcodec_open2(codecCtx, codec, &dict) < 0) {
		Log::msg(Log::Warning, "Module LibavDecode: Couldn't open stream");
		throw std::runtime_error("[LibavDecode] Couldn't open stream.");
	}

	auto props = new PropsDecoder(codecCtx);

	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO: videoPin = addPin(new PinPicture(props)); break;
	case AVMEDIA_TYPE_AUDIO: audioPin = addPin(new PinPcm(props)); break;
	default: throw std::runtime_error("[LibavDecode] Invalid Pin type.");
	}
}

LibavDecode::~LibavDecode() {
	avcodec_close(codecCtx);
	av_free(codecCtx);
}

void LibavDecode::processAudio(const DataAVPacket *data) {
	AVPacket *pkt = data->getPacket();
	auto out = audioPin->getBuffer(0);

	int gotFrame;
	if (avcodec_decode_audio4(codecCtx, avFrame->get(), &gotFrame, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding audio.");
		return;
	}
	if (gotFrame) {
		//TODO: not supposed to change across the session but the pin doesn't hold the right type
		PcmFormat pcmFormat;
		libavFrame2pcmConvert(avFrame->get(), &pcmFormat);
		out->setFormat(pcmFormat);
		for (uint8_t i = 0; i < pcmFormat.numPlanes; ++i) {
			out->setPlane(i, avFrame->get()->data[i], avFrame->get()->linesize[0] / pcmFormat.numPlanes);
		}
		setTimestamp(out, avFrame->get()->nb_samples);
		pins[0]->emit(out);
		++m_numFrames;
	}
}

namespace {
void copyToPicture(AVFrame const* avFrame, Picture* pic) {
	pic->setResolution(Resolution(avFrame->width, avFrame->height));

	for (int comp=0; comp<3; ++comp) {
		auto subsampling = comp == 0 ? 1 : 2;
		auto src = avFrame->data[comp];
		auto srcPitch = avFrame->linesize[comp];

		auto dst = pic->getComp(comp);
		auto dstPitch = pic->getPitch(comp);

		auto const h = avFrame->height / subsampling;
		auto const w = avFrame->width / subsampling;

		for (int y=0; y<h; ++y) {
			memcpy(dst, src, w);
			src += srcPitch;
			dst += dstPitch;
		}
	}
}
}

void LibavDecode::processVideo(const DataAVPacket *decoderData) {
	AVPacket *pkt = decoderData->getPacket();
	auto pic = videoPin->getBuffer(0);
	int gotPicture;
	if (avcodec_decode_video2(codecCtx, avFrame->get(), &gotPicture, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding video.");
		return;
	}
	if (gotPicture) {
		copyToPicture(avFrame->get(), pic.get());
		setTimestamp(pic);
		pins[0]->emit(pic);
		++m_numFrames;
	}
}

void LibavDecode::setTimestamp(std::shared_ptr<Data> s, uint64_t increment) const {
	uint64_t t;
	if (m_numFrames == 0) {
		t = 0;
	} else if (codecCtx->time_base.den == 0) {
		throw std::runtime_error("[LibavDecode] Unknown frame rate. Cannot set the timestamp.");
	} else {
		t = (m_numFrames * increment * IClock::Rate * codecCtx->time_base.num * codecCtx->ticks_per_frame + (codecCtx->time_base.den / 2)) / codecCtx->time_base.den;
	}
	s->setTime(t);
}

void LibavDecode::process(std::shared_ptr<const Data> data) {
	auto decoderData = safe_cast<const DataAVPacket>(data);
	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		return processVideo(decoderData.get());
		break;
	case AVMEDIA_TYPE_AUDIO:
		return processAudio(decoderData.get());
		break;
	default:
		assert(0);
		return;
	}
}

}
