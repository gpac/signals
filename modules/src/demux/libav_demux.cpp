#include "libav_demux.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "../common/libav.hpp"
#include <cassert>
#include <string>

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

	std::vector<Pin<>*> signals;
	for (unsigned i = 0; i<formatCtx->nb_streams; i++) {
		signals.push_back(new Pin<>(new PropsDecoder(formatCtx->streams[i]->codec)));
	}

	return new LibavDemux(formatCtx, signals);
}

LibavDemux::LibavDemux(struct AVFormatContext *formatCtx, std::vector<Pin<>*> signals)
: formatCtx(formatCtx) {
	for (size_t i = 0; i < signals.size(); ++i) {
		this->signals.push_back(uptr(signals[i]));
	}
}

LibavDemux::~LibavDemux() {
	avformat_close_input(&formatCtx);
}

bool LibavDemux::process(std::shared_ptr<Data> data) {
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
	pd.buf = new unsigned char[size + AVPROBE_PADDING_SIZE];
	pd.filename = NULL;

	FILE *f = fopen(url.c_str(), "rb");
	if (!f) {
		Log::msg(Log::Info, "[LibavDemux] Couldn't open file %s for probing. Aborting.", url.c_str());
		delete[] pd.buf;
		return false;
	}
	size_t bytesRead = fread(pd.buf, 1, size, f);
	pd.buf_size = (int)bytesRead;
	fclose(f);
	if (bytesRead < size) {
		Log::msg(Log::Warning, "[LibavDemux] Could only read %lu bytes (instead of %lu) for probing format.", (unsigned long)bytesRead, (unsigned long)size);
	}
	memset(pd.buf + bytesRead, 0, AVPROBE_PADDING_SIZE);

	avcodec_register_all();
	av_register_all();
	avformat_network_init();

	AVInputFormat *format = av_probe_input_format(&pd, 1);
	delete[] pd.buf;
	if (!format)
		return false;
	else
		return true;
}

}
