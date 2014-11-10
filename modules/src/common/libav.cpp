#include "libav.hpp"
#include "pcm.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include <cassert>
#include <cstdio>
#include <string.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace {
Log::Level avLogLevel(int level) {
	switch (level) {
	case AV_LOG_QUIET:
		return Log::Quiet;
	case AV_LOG_PANIC:
	case AV_LOG_FATAL:
	case AV_LOG_ERROR:
		return Log::Warning;
	case AV_LOG_WARNING:
		return Log::Info;
	case AV_LOG_INFO:
	case AV_LOG_VERBOSE:
	case AV_LOG_DEBUG:
		return Log::Debug;
	default:
		assert(0);
		return Log::Debug;
	}
}

const char* avlogLevelName(int level) {
	switch (level) {
	case AV_LOG_QUIET:
		return "quiet";
	case AV_LOG_PANIC:
		return "panic";
	case AV_LOG_FATAL:
		return "fatal";
	case AV_LOG_ERROR:
		return "error";
	case AV_LOG_WARNING:
		return "warning";
	case AV_LOG_INFO:
		return "info";
	case AV_LOG_VERBOSE:
		return "verbose";
	case AV_LOG_DEBUG:
		return "debug";
	default:
		assert(0);
		return "unknown";
	}
}

void libavAudioCtxConvertGeneric(const Modules::PcmFormat *cfg, int &sampleRate, AVSampleFormat &format, int &numChannels, uint64_t &layout) {
	sampleRate = cfg->getSampleRate();

	switch (cfg->getFormat()) {
	case Modules::S16: format = AV_SAMPLE_FMT_S16; break;
	case Modules::F32: format = AV_SAMPLE_FMT_FLT; break;
	default: throw std::runtime_error("Unknown libav audio format");
	}

	numChannels = cfg->getNumChannels();
	switch (cfg->getLayout()) {
	case Modules::Mono: layout = AV_CH_LAYOUT_MONO; break;
	case Modules::Stereo: layout = AV_CH_LAYOUT_STEREO; break;
	default: throw std::runtime_error("Unknown libav audio layout");
	}
}
}

namespace Modules {

void libavAudioCtxConvert(const PcmFormat *cfg, AVCodecContext *codecCtx) {
	libavAudioCtxConvertGeneric(cfg, codecCtx->sample_rate, codecCtx->sample_fmt, codecCtx->channels, codecCtx->channel_layout);
}

void libavFrame2pcmConvert(const AVFrame *frame, PcmFormat *cfg) {
	cfg->setSampleRate(frame->sample_rate);

	switch (frame->format) {
	case AV_SAMPLE_FMT_S16:
	case AV_SAMPLE_FMT_S16P:
		cfg->setFormat(Modules::S16);
		break;
	case AV_SAMPLE_FMT_FLT:  
	case AV_SAMPLE_FMT_FLTP:
		cfg->setFormat(Modules::F32);
		break;
	default:
		throw std::runtime_error("Unknown libav audio format");
	}

	cfg->setNumPlanes(frame->channels);
	switch (frame->channel_layout) {
	case AV_CH_LAYOUT_MONO:   cfg->setLayout(Modules::Mono); break;
	case AV_CH_LAYOUT_STEREO: cfg->setLayout(Modules::Stereo); break;
	default: throw std::runtime_error("Unknown libav audio layout");
	}
}

void libavFrameDataConvert(const PcmData *data, AVFrame *frame) {
	libavAudioCtxConvertGeneric(data, frame->sample_rate, (AVSampleFormat&)frame->format, frame->channels, frame->channel_layout);
	for (size_t i = 0; i < data->getNumPlanes(); ++i) {
		frame->data[i] = data->getPlane(i);
		frame->linesize[i] = (int)data->getPlaneSize(i);
	}
	frame->nb_samples = (int)(data->getPlaneSize(0) / data->getBytesPerSample());
}

DataAVPacket::DataAVPacket(size_t size)
	: pkt(new AVPacket) {
	av_init_packet(pkt.get());
	av_free_packet(pkt.get());
	if (size)
		av_new_packet(pkt.get(), (int)size);
}

DataAVPacket::~DataAVPacket() {
	Log::msg(Log::Debug, "Freeing %s, pts=%s", this, pkt->pts);
	av_free_packet(pkt.get());
}

uint8_t* DataAVPacket::data() {
	return pkt->data;
}

uint64_t DataAVPacket::size() const {
	return pkt->size;
}

AVPacket* DataAVPacket::getPacket() const {
	return pkt.get();
}

void DataAVPacket::resize(size_t /*size*/) {
	assert(0);
}

DataAVFrame::DataAVFrame(size_t size)
	: frame(av_frame_alloc()) {
}

DataAVFrame::~DataAVFrame() {
	av_frame_free(&frame);
}

uint8_t* DataAVFrame::data() {
	return frame->data[0];
}

uint64_t DataAVFrame::size() const {
	if (frame->linesize[1])
		Log::msg(Log::Debug, "[DataAVFrame] Storage is planar. Returning the sum of all plane size.");
	return avpicture_get_size((AVPixelFormat)frame->format, frame->width, frame->height);
}

AVFrame* DataAVFrame::getFrame() const {
	return frame;
}

void DataAVFrame::resize(size_t /*size*/) {
	assert(0);
}

void buildAVDictionary(const std::string &moduleName, AVDictionary **dict, const char *options, const char *type) {
	auto opt = stringDup(options);
	char *tok = strtok(opt.data(), "- ");
	char *tokval = NULL;
	while (tok && (tokval = strtok(NULL, "- "))) {
		if (av_dict_set(dict, tok, tokval, 0) < 0) {
			Log::msg(Log::Warning, "[%s] unknown %s option \"%s\" with value \"%s\"", moduleName.c_str(), type, tok, tokval);
		}
		tok = strtok(NULL, "- ");
	}
}

void avLog(void* /*avcl*/, int level, const char *fmt, va_list vl) {
#if defined(__CYGWIN__) // cygwin does not have vsnprintf in std=c++11 mode. To be removed when cygwin is fixed
	Log::msg(avLogLevel(level), "[libav-log::%s] %s", avlogLevelName(level), fmt);
#else
	char buffer[1024];
	std::vsnprintf(buffer, sizeof(buffer)-1, fmt, vl);

	// remove trailing end of line
	{
		auto const N = strlen(buffer);
		if(N > 0 && buffer[N-1] == '\n')
			buffer[N-1] = 0;
	}
	Log::msg(avLogLevel(level), "[libav-log::%s] %s", avlogLevelName(level), buffer);
#endif
}

Pin* PinLibavPacketFactory::createPin(IProps *props) {
	return new PinLibavPacket(props);
}

Pin* PinLibavFrameFactory::createPin(IProps *props) {
	auto p = dynamic_cast<PropsDecoder*>(props);
	if (!p)
		throw std::runtime_error("dynamic cast error");

	switch (p->getAVCodecContext()->codec_type) {
	case AVMEDIA_TYPE_VIDEO: return new PinLibavFrame(props);
	case AVMEDIA_TYPE_AUDIO: return new PinPcm(props);
	default: throw std::runtime_error("[PinLibavFrameFactory] Invalid Pin type.");
}
	
}

}
