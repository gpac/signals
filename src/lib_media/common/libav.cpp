#include "libav.hpp"
#include "pcm.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include <cassert>
#include <cstdio>
#include <string.h>

extern "C" {
#include <libavcodec/avcodec.h>
#undef PixelFormat
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
}

namespace Modules {

void libavAudioCtxConvertLibav(const Modules::PcmFormat *cfg, int &sampleRate, int &format, int &numChannels, uint64_t &layout) {
	sampleRate = cfg->sampleRate;

	switch (cfg->layout) {
	case Modules::Mono: layout = AV_CH_LAYOUT_MONO; break;
	case Modules::Stereo: layout = AV_CH_LAYOUT_STEREO; break;
	default: throw std::runtime_error("Unknown libav audio layout");
	}
	numChannels = av_get_channel_layout_nb_channels(layout);
	assert(numChannels == cfg->numChannels);

	switch (cfg->sampleFormat) {
	case Modules::S16: format = av_get_alt_sample_fmt(AV_SAMPLE_FMT_S16, cfg->numPlanes > 1); break;
	case Modules::F32: format = av_get_alt_sample_fmt(AV_SAMPLE_FMT_FLT, cfg->numPlanes > 1); break;
	default: throw std::runtime_error("Unknown libav audio format");
	}
}

void libavAudioCtxConvert(const PcmFormat *cfg, AVCodecContext *codecCtx) {
	libavAudioCtxConvertLibav(cfg, codecCtx->sample_rate, (int&)codecCtx->sample_fmt, codecCtx->channels, codecCtx->channel_layout);
}

void libavFrame2pcmConvert(const AVFrame *frame, PcmFormat *cfg) {
	cfg->sampleRate = frame->sample_rate;

	cfg->numChannels = cfg->numPlanes = frame->channels;
	switch (frame->format) {
	case AV_SAMPLE_FMT_S16: cfg->numPlanes = 1;
	case AV_SAMPLE_FMT_S16P:
		cfg->sampleFormat = Modules::S16;
		break;
	case AV_SAMPLE_FMT_FLT: cfg->numPlanes = 1;
	case AV_SAMPLE_FMT_FLTP:
		cfg->sampleFormat = Modules::F32;
		break;
	default:
		throw std::runtime_error("Unknown libav audio format");
	}

	switch (frame->channel_layout) {
	case AV_CH_LAYOUT_MONO:   cfg->layout = Modules::Mono; break;
	case AV_CH_LAYOUT_STEREO: cfg->layout = Modules::Stereo; break;
	default: throw std::runtime_error("Unknown libav audio layout");
	}
}

void libavFrameDataConvert(const PcmData *pcmData, AVFrame *frame) {
	auto const& format = pcmData->getFormat();
	libavAudioCtxConvertLibav(&format, frame->sample_rate, frame->format, frame->channels, frame->channel_layout);
	for (size_t i = 0; i < format.numPlanes; ++i) {
		frame->data[i] = pcmData->getPlane(i);
		frame->linesize[i] = (int)pcmData->getPlaneSize(i);
	}
	frame->nb_samples = (int)(pcmData->size() / format.getBytesPerSample());
}

void pixelFormat2libavPixFmt(const enum PixelFormat format, AVPixelFormat &avPixfmt) {
	switch (format) {
	case YUV420P: avPixfmt = AV_PIX_FMT_YUV420P; break;
	case YUYV422: avPixfmt = AV_PIX_FMT_YUYV422; break;
	case RGB24: avPixfmt = AV_PIX_FMT_RGB24; break;
	default:
		throw std::runtime_error("Unknown pixel format to convert (1). Please contact your vendor.");
	}
}

enum PixelFormat libavPixFmt2PixelFormat(const AVPixelFormat &avPixfmt) {
	switch (avPixfmt) {
	case AV_PIX_FMT_YUV420P: return YUV420P;
	case AV_PIX_FMT_YUYV422: return YUYV422;
	case AV_PIX_FMT_RGB24: return RGB24;
	default:
		throw std::runtime_error("Unknown pixel format to convert (2). Please contact your vendor.");
	}
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

uint8_t const* DataAVPacket::data() const {
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

void buildAVDictionary(const std::string &moduleName, AVDictionary **dict, const char *options, const char *type) {
	auto opt = stringDup(options);
	char *tok = strtok(opt.data(), "- ");
	char *tokval = nullptr;
	while (tok && (tokval = strtok(nullptr, "- "))) {
		if (av_dict_set(dict, tok, tokval, 0) < 0) {
			Log::msg(Log::Warning, "[%s] unknown %s option \"%s\" with value \"%s\"", moduleName.c_str(), type, tok, tokval);
		}
		tok = strtok(nullptr, "- ");
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

}

