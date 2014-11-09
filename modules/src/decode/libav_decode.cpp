#include "libav_decode.hpp"
#include "internal/core/clock.hpp"
#include "../common/pcm.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include <cassert>

#include "ffpp.hpp"

namespace {
auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);
}

namespace Decode {

LibavDecode::LibavDecode(const PropsDecoder &props)
	: Module(new PinLibavFrameFactory), codecCtx(avcodec_alloc_context3(NULL)), avFrame(new ffpp::Frame), m_numFrames(0) {
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

	signals.push_back(uptr(pinFactory->createPin(new PropsDecoder(codecCtx))));
}

LibavDecode::~LibavDecode() {
	avcodec_close(codecCtx);
	av_free(codecCtx);
}

void LibavDecode::processAudio(const DataAVPacket *data) {
	AVPacket *pkt = data->getPacket();
	auto out = safe_cast<PcmData>(signals[0]->getBuffer(0));
	//Romain: utile? libavFrameDataConvert(out.get(), avFrame->get());

	int gotFrame;
	if (avcodec_decode_audio4(codecCtx, avFrame->get(), &gotFrame, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding audio.");
		return;
	}
	if (gotFrame) {
		for (uint8_t i = 0; i < out->getNumPlanes(); ++i) {
			out->setPlane(i, avFrame->get()->data[i], avFrame->get()->linesize[i]);
		}
		setTimestamp(out, avFrame->get()->nb_samples);
		signals[0]->emit(out);
		++m_numFrames;
	}
}

void LibavDecode::processVideo(const DataAVPacket *decoderData) {
	AVPacket *pkt = decoderData->getPacket();
	auto out = safe_cast<DataAVFrame>(signals[0]->getBuffer(0));
	int gotPicture;
	if (avcodec_decode_video2(codecCtx, out->getFrame(), &gotPicture, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding video.");
		return;
	}
	if (gotPicture) {
		setTimestamp(out);
		signals[0]->emit(out);
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

void LibavDecode::process(std::shared_ptr<Data> data) {
	auto decoderData = safe_cast<DataAVPacket>(data);
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
