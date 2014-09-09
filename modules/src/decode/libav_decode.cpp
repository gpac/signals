#include "libav_decode.hpp"
#include "internal/clock.hpp"
#include "../common/pcm.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include <cassert>
#include <string>

#include "ffpp.hpp"
#include "converters.hpp"

namespace {
auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);
}

namespace Decode {

LibavDecode* LibavDecode::create(const PropsDecoder &props) {
	return new LibavDecode(props.getAVCodecContext());
}

LibavDecode::LibavDecode(AVCodecContext *codecCtx2)
	: Module(new PinPcmFactory), codecCtx(new AVCodecContext), avFrame(new ffpp::Frame), m_numFrames(0) {
	*codecCtx = *codecCtx2;

	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
	case AVMEDIA_TYPE_AUDIO:
		break;
	default:
		Log::msg(Log::Warning, "Module LibavDecode: codec_type %s not supported. Must be audio or video.", codecCtx->codec_type);
		throw std::runtime_error("Unknown decoder type. Failed.");
	}

	//find an appropriate decoder
	auto codec = avcodec_find_decoder(codecCtx->codec_id);
	if (!codec) {
		Log::msg(Log::Warning, "Module LibavDecode: Codec not found");
		throw std::runtime_error("Decoder not found.");
	}

	//TODO: test: force single threaded as h264 probing seems to miss SPS/PPS and seek fails silently
	ffpp::Dict dict;
	dict.set("threads", "1");

	//open the codec
	if (avcodec_open2(codecCtx.get(), codec, &dict) < 0) {
		Log::msg(Log::Warning, "Module LibavDecode: Couldn't open stream");
		throw std::runtime_error("Couldn't open stream.");
	}

	signals.push_back(uptr(pinFactory->createPin()));
}

LibavDecode::~LibavDecode() {
	avcodec_close(codecCtx.get());
}

bool LibavDecode::processAudio(DataAVPacket *decoderData) {
	AVPacket *pkt = decoderData->getPacket();
	int gotFrame;
	if (avcodec_decode_audio4(codecCtx.get(), avFrame->get(), &gotFrame, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding audio.");
		return true;
	}
	if (gotFrame) {
		if(!m_pAudioConverter)
			m_pAudioConverter.reset(new AudioConverter(*codecCtx, *signals[0]));

		auto out = m_pAudioConverter->convert(codecCtx.get(), avFrame->get());
		signals[0]->emit(out);
	}

	return true;
}

bool LibavDecode::processVideo(DataAVPacket *decoderData) {
	AVPacket *pkt = decoderData->getPacket();
	int gotPicture;
	if (avcodec_decode_video2(codecCtx.get(), avFrame->get(), &gotPicture, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding video.");
		return true;
	}
	if (gotPicture) {
		if(!m_pVideoConverter)
			m_pVideoConverter.reset(new VideoConverter(*codecCtx, *signals[0]));

		auto out = m_pVideoConverter->convert(codecCtx.get(), avFrame->get());
		setTimestamp(out);
		signals[0]->emit(out);
		++m_numFrames;
	}
	return true;
}

void LibavDecode::setTimestamp(std::shared_ptr<Data> s) const {
	auto const framePeriodIn180k = IClock::Rate / 24;
	s->setTime(m_numFrames * framePeriodIn180k);
}

bool LibavDecode::process(std::shared_ptr<Data> data) {
	auto decoderData = dynamic_cast<DataAVPacket*>(data.get());
	if (!decoderData) {
		Log::msg(Log::Warning, "[LibavDecode] Invalid packet type.");
		return false;
	}
	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		return processVideo(decoderData);
		break;
	case AVMEDIA_TYPE_AUDIO:
		return processAudio(decoderData);
		break;
	default:
		assert(0);
		return false;
	}
}

bool LibavDecode::handles(const std::string &url) {
	return LibavDecode::canHandle(url);
}

bool LibavDecode::canHandle(const std::string &/*url*/) {
	return true; //TODO
}

}
