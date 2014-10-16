#include "libav_demux.hpp"
#include "internal/core/clock.hpp"
#include "../common/libav.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include <cassert>
#include <string>
#include <fstream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

namespace {
auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);
}

namespace Demux {

LibavDemux* LibavDemux::create(const std::string &url) {
	struct AVFormatContext *formatCtx = NULL;
	if (!(formatCtx = avformat_alloc_context())) {
		Log::msg(Log::Warning, "Module LibavDemux: Can't allocate format context");
		throw std::runtime_error("Format Context allocation failed.");
	}

	if (avformat_open_input(&formatCtx, url.c_str(), NULL, NULL))  {
		Log::msg(Log::Warning, "Module LibavDemux: Error when initializing the format context");
		avformat_close_input(&formatCtx);
		throw std::runtime_error("Format Context init failed.");
	}

	//if you don't call you may miss the first frames
	if (avformat_find_stream_info(formatCtx, NULL) < 0) {
		Log::msg(Log::Warning, "Module LibavDemux: Couldn't get additional video stream info");
		avformat_close_input(&formatCtx);
		throw std::runtime_error("Couldn't find stream info.");
	}

	return new LibavDemux(formatCtx);
}

LibavDemux::LibavDemux(struct AVFormatContext *formatCtx)
	: Module(new PinLibavPacketFactory), m_formatCtx(formatCtx) {
	for (unsigned i = 0; i<formatCtx->nb_streams; i++) {
		signals.push_back(uptr(pinFactory->createPin(new PropsDecoder(formatCtx->streams[i]->codec))));
	}
}

LibavDemux::~LibavDemux() {
	avformat_close_input(&m_formatCtx);
}

void LibavDemux::process(std::shared_ptr<Data> /*data*/) {
	for (;;) {
		auto out = std::dynamic_pointer_cast<DataAVPacket>(signals[0/*FIXME: pkt->stream_index*/]->getBuffer(0));
		AVPacket *pkt = out->getPacket();
		int status = av_read_frame(m_formatCtx, pkt);
		if (status < 0) {
			if (status == (int)AVERROR_EOF || (m_formatCtx->pb && m_formatCtx->pb->eof_reached)) {
			} else if (m_formatCtx->pb && m_formatCtx->pb->error) {
				Log::msg(Log::Warning, "[Libavcodec_55] Stream contains an irrecoverable error - leaving");
			}
			return;
		}

		auto const base = m_formatCtx->streams[pkt->stream_index]->time_base;
		auto const time = pkt->pts * base.num * IClock::Rate / base.den;
		out->setTime(time);
		signals[pkt->stream_index]->emit(out);
	}
}

}
