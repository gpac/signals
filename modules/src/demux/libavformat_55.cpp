#include "libavformat_55.hpp"
#include "../utils/log.hpp"
#include "../common/libav.hpp"
#include <cassert>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

namespace Demux {

Libavformat_55* Libavformat_55::create(const std::string &url) {
	struct AVFormatContext *formatCtx = NULL;

	avcodec_register_all();
	av_register_all();
	//TODO: custom log: av_log_set_callback(avlog);

	if (!(formatCtx = avformat_alloc_context())) {
		Log::msg(Log::Warning, "Module Libavformat_55: Can't allocate format context");
		return NULL;
	}

	if (avformat_open_input(&formatCtx, url.c_str(), NULL, NULL))  {
		Log::msg(Log::Warning, "Module Libavformat_55: Error when initializing the format context");
		avformat_close_input(&formatCtx);
		return NULL;
	}

	//if you don't call you may miss the first frames
	if (avformat_find_stream_info(formatCtx, NULL) < 0) {
		Log::msg(Log::Warning, "Module Libavformat_55: Couldn't get additional video stream info");
		avformat_close_input(&formatCtx);
		return NULL;
	}

	std::vector<Pin<>*> signals;
	for (unsigned i = 0; i<formatCtx->nb_streams; i++) {
		signals.push_back(new Pin<>(new PropsDecoder(formatCtx->streams[i]->codec)));
	}

	return new Libavformat_55(formatCtx, signals);
}

Libavformat_55::Libavformat_55(struct AVFormatContext *formatCtx, std::vector<Pin<>*> signals)
: formatCtx(formatCtx) {
	for (size_t i = 0; i < signals.size(); ++i) {
		this->signals.push_back(signals[i]);
	}
}

Libavformat_55::~Libavformat_55() {
	for (unsigned i = 0; i < formatCtx->nb_streams; i++) {
		delete signals[i];
	}
	avformat_close_input(&formatCtx);
}

bool Libavformat_55::process(std::shared_ptr<Data> data) {
	std::shared_ptr<DataDecoder> out(new DataDecoder);
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

bool Libavformat_55::handles(const std::string &url) {
	return Libavformat_55::canHandle(url);
}

bool Libavformat_55::canHandle(const std::string &url) {
	//FIXME: only works for files
	AVProbeData pd;
	const size_t size = 1024;
	pd.buf = new unsigned char[size + AVPROBE_PADDING_SIZE];
	pd.filename = NULL;

	FILE *f = fopen(url.c_str(), "rb");
	if (!f) {
		Log::msg(Log::Info, "[Libavformat_55] Couldn't open file %s for probing. Aborting.", url.c_str());
		delete[] pd.buf;
		return false;
	}
	size_t bytesRead = fread(pd.buf, 1, size, f);
	pd.buf_size = (int)bytesRead;
	fclose(f);
	if (bytesRead < size) {
		Log::msg(Log::Warning, "[Libavformat_55] Could only read %lu bytes (instead of %lu) for probing format.", (unsigned long)bytesRead, (unsigned long)size);
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
