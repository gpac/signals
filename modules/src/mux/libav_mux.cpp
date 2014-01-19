#include "libav_mux.hpp"
#include "../utils/log.hpp"
#include "../common/libav.hpp"
#include <cassert>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

namespace {
	//Romain: remove
void clear(AVFormatContext *formatCtx, AVStream *videoStream, AVFrame *frame, AVPacket *pkt) {
	if (videoStream && videoStream->codec) {
		avcodec_close(videoStream->codec);
	}
	if (frame) {
		avcodec_free_frame(&frame);
	}
	if (formatCtx && !(formatCtx->flags & AVFMT_NOFILE)) {
		avio_close(formatCtx->pb); //close output file
	}
	if (formatCtx) {
		avformat_free_context(formatCtx);
	}
	delete pkt;
}
}

namespace Mux {

LibavMux* LibavMux::create(const std::string &baseName) {
	AVFormatContext *formatCtx = NULL;
	AVStream *videoStream = NULL; //Romain: useless
	AVFrame *avFrame = NULL;
	AVPacket *avPkt = NULL;
	uint8_t *avOutputBuffer = NULL;

	av_register_all();
	avformat_network_init();
	//TODO: custom log: av_log_set_callback(avlog);

	/* parse the format optionsDict */
	std::string optionsStr = "-format mp4"; //Romain TODO
	AVDictionary *optionsDict = NULL;
	buildAVDictionary("[libav_mux]", &optionsDict, optionsStr.c_str(), "format");

	/* setup container */
	AVOutputFormat *of = av_guess_format(av_dict_get(optionsDict, "format", NULL, 0)->value, NULL, NULL);
	if (!of) {
		Log::msg(Log::Warning, "[libav_mux] couldn't guess container from file extension");
		av_dict_free(&optionsDict);
		delete[] avOutputBuffer;
		clear(formatCtx, videoStream, avFrame, avPkt);
		return NULL;
	}
	av_dict_free(&optionsDict);

	/* output format context */
	formatCtx = avformat_alloc_context();
	if (!formatCtx) {
		Log::msg(Log::Warning, "[libav_mux] format context couldn't be allocated.");
		delete[] avOutputBuffer;
		clear(formatCtx, videoStream, avFrame, avPkt);
		return NULL;
	}
	formatCtx->oformat = of;

	std::stringstream fileName;
	fileName << baseName;
	std::stringstream formatExts(of->extensions); //get the first extension recommended by ffmpeg
	std::string fileNameExt;
	std::getline(formatExts, fileNameExt, ',');
	fileName << "." << fileNameExt;
	
	/* open the output file, if needed */
	if (!(formatCtx->flags & AVFMT_NOFILE)) {
		if (avio_open(&formatCtx->pb, fileName.str().c_str(), AVIO_FLAG_READ_WRITE) < 0) {
			Log::msg(Log::Warning, "[libav_mux] could not open %s, disable output.", baseName);
			delete[] avOutputBuffer;
			clear(formatCtx, videoStream, avFrame, avPkt);
			return NULL;
		}
		strncpy(formatCtx->filename, fileName.str().c_str(), sizeof(formatCtx->filename));
	}

	return new LibavMux(formatCtx);
}

LibavMux::LibavMux(struct AVFormatContext *formatCtx)
: formatCtx(formatCtx), headerWritten(false) {
}

LibavMux::~LibavMux() {
	if (formatCtx) {
		av_write_trailer(formatCtx); //write the trailer if any
	}
	if (formatCtx && !(formatCtx->flags & AVFMT_NOFILE)) {
		avio_close(formatCtx->pb); //close output file
	}
	if (formatCtx) {
		avformat_free_context(formatCtx);
	}
}

void LibavMux::ensureHeader() {
	if (!headerWritten) {
		if (avformat_write_header(formatCtx, NULL) != 0) {
			Log::msg(Log::Warning, "[libav_mux] fatal error: can't write the container header");
			for (unsigned i = 0; i < formatCtx->nb_streams; i++) {
				if (formatCtx->streams[i]->codec && formatCtx->streams[i]->codec->codec) {
					Log::msg(Log::Debug, "[libav_mux] codec[%u] is \"%s\" (%s)", i, formatCtx->streams[i]->codec->codec->name, formatCtx->streams[i]->codec->codec->long_name);
				}
			}
		}
		headerWritten = true;
	}
}

bool LibavMux::process(std::shared_ptr<Data> data) {
	ensureHeader();
	assert(0); //Romain: TODO
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

bool LibavMux::handles(const std::string &url) {
	return LibavMux::canHandle(url);
}

bool LibavMux::canHandle(const std::string &url) {
	return true; //TODO
}

}
