#include "libav_demux.hpp"
#include "../common/libav.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include <cassert>
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

LibavDemux::LibavDemux(const std::string &url) {
	if (!(m_formatCtx = avformat_alloc_context())) {
		Log::msg(Log::Warning, "[LibavDemux] Can't allocate format context");
		throw std::runtime_error("Format Context allocation failed.");
	}

	if (avformat_open_input(&m_formatCtx, url.c_str(), NULL, NULL)) {
		Log::msg(Log::Warning, "[LibavDemux] Error when opening input '%s'", url);
		if (m_formatCtx) avformat_close_input(&m_formatCtx);
		throw std::runtime_error("Format Context init failed.");
	}

	//if you don't call you may miss the first frames
	if (avformat_find_stream_info(m_formatCtx, NULL) < 0) {
		Log::msg(Log::Warning, "[LibavDemux] Couldn't get additional video stream info");
		avformat_close_input(&m_formatCtx);
		throw std::runtime_error("Couldn't find stream info.");
	}

	for (unsigned i = 0; i<m_formatCtx->nb_streams; i++) {
		auto props = new PropsDecoder(m_formatCtx->streams[i]->codec);
		outputs.push_back(addPin(new PinDataDefault<DataAVPacket>(props)));
	}
}

LibavDemux::~LibavDemux() {
	avformat_close_input(&m_formatCtx);
}

void LibavDemux::process(std::shared_ptr<const Data> /*data*/) {
	for (;;) {
		auto out = outputs[0]->getBuffer(0);
		AVPacket *pkt = out->getPacket();
		int status = av_read_frame(m_formatCtx, pkt);
		if (status < 0) {
			if (status == (int)AVERROR_EOF || (m_formatCtx->pb && m_formatCtx->pb->eof_reached)) {
			} else if (m_formatCtx->pb && m_formatCtx->pb->error) {
				Log::msg(Log::Warning, "[LibavDemux] Stream contains an irrecoverable error - leaving");
			}
			return;
		}

		auto const base = m_formatCtx->streams[pkt->stream_index]->time_base;
		auto const time = timescaleToClock(pkt->pts * base.num, base.den);
		out->setTime(time);
		outputs[pkt->stream_index]->emit(out);
	}
}

}
