#include "libav_demux.hpp"
#include "internal/clock.hpp"
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
	: formatCtx(formatCtx) {
	for (unsigned i = 0; i<formatCtx->nb_streams; i++) {
		signals.push_back(uptr(pinFactory->createPin(new PropsDecoder(formatCtx->streams[i]->codec))));
	}
}

LibavDemux::~LibavDemux() {
	avformat_close_input(&formatCtx);
}

bool LibavDemux::process(std::shared_ptr<Data> /*data*/) {
	std::shared_ptr<DataAVPacket> out(new DataAVPacket);
	AVPacket *pkt = out->getPacket();
	int status = av_read_frame(formatCtx, pkt);
	if (status < 0) {
		if (status == (int)AVERROR_EOF || (formatCtx->pb && formatCtx->pb->eof_reached)) {
		} else if (formatCtx->pb && formatCtx->pb->error) {
			Log::msg(Log::Warning, "[Libavcodec_55] Stream contains an irrecoverable error - leaving");
		}
		return false;
	}

	auto const base = formatCtx->streams[pkt->stream_index]->time_base;
	auto const time = pkt->pts * base.num * IClock::Rate / base.den;
	out->setTime(time);
	signals[pkt->stream_index]->emit(out);
	return true;
}

bool LibavDemux::handles(const std::string &url) {
	return LibavDemux::canHandle(url);
}

bool LibavDemux::canHandle(const std::string &url) {
	//FIXME: only works for files
	AVProbeData pd;
	const size_t size = 1024;
	std::vector<uint8_t> buf(size + AVPROBE_PADDING_SIZE);
	pd.buf = buf.data();
	pd.filename = NULL;

	std::ifstream fp(url, std::ios::binary);
	if (!fp.is_open()) {
		Log::msg(Log::Info, "[LibavDemux] Couldn't open file %s for probing. Aborting.", url.c_str());
		return false;
	}

	fp.read((char*)pd.buf, size);
	const size_t bytesRead = (size_t)fp.gcount();
	pd.buf_size = (int)bytesRead;
	if (bytesRead < size) {
		Log::msg(Log::Warning, "[LibavDemux] Could only read %lu bytes (instead of %lu) for probing format.", (unsigned long)bytesRead, (unsigned long)size);
	}
	memset(pd.buf + bytesRead, 0, AVPROBE_PADDING_SIZE);

	avformat_network_init();

	AVInputFormat *format = av_probe_input_format(&pd, 1);
	if (!format)
		return false;

	return true;
}

}
