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

namespace Mux {

LibavMux* LibavMux::create(const std::string &baseName) {
	AVFormatContext *formatCtx = NULL;

	av_register_all();
	avformat_network_init();
	av_log_set_callback(avLog);

	/* parse the format optionsDict */
	std::string optionsStr = "-format mp4"; //TODO
	AVDictionary *optionsDict = NULL;
	buildAVDictionary("[libav_mux]", &optionsDict, optionsStr.c_str(), "format");

	/* setup container */
	AVOutputFormat *of = av_guess_format(av_dict_get(optionsDict, "format", NULL, 0)->value, NULL, NULL);
	if (!of) {
		Log::msg(Log::Warning, "[libav_mux] couldn't guess container from file extension");
		av_dict_free(&optionsDict);
		return NULL;
	}
	av_dict_free(&optionsDict);

	/* output format context */
	formatCtx = avformat_alloc_context();
	if (!formatCtx) {
		Log::msg(Log::Warning, "[libav_mux] format context couldn't be allocated.");
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
			avformat_free_context(formatCtx);
			return NULL;
		}
		strncpy(formatCtx->filename, fileName.str().c_str(), sizeof(formatCtx->filename));
	}

	return new LibavMux(formatCtx);
}

LibavMux::LibavMux(struct AVFormatContext *formatCtx)
: formatCtx(formatCtx), headerWritten(false) {
	signals.push_back(new Pin<>(new PropsMuxer(formatCtx))); //FIXME: we create the pin only for the props...
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
	delete signals[0];
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
	DataAVPacket *encoderData = dynamic_cast<DataAVPacket*>(data.get());
	if (!encoderData) {
		return false;
	}
	AVPacket *pkt = encoderData->getPacket();

	ensureHeader();
	
	/* Timestamps */
	assert(pkt->pts != (int64_t)AV_NOPTS_VALUE);
	AVStream *avStream = formatCtx->streams[0]; //FIXME: fixed '0' for stream num: this is not a mux yet ;)
	pkt->dts = av_rescale_q(pkt->dts, avStream->codec->time_base, avStream->time_base);
	pkt->pts = av_rescale_q(pkt->pts, avStream->codec->time_base, avStream->time_base);
	pkt->duration = (int)av_rescale_q(pkt->duration, avStream->codec->time_base, avStream->time_base);

	/* write the compressed frame to the container output file */
	pkt->stream_index = avStream->index;
	if (av_interleaved_write_frame(formatCtx, pkt) != 0) {
		Log::msg(Log::Warning, "[libav_mux] can't write video frame.");
		return false;
	}

	return true;
}

bool LibavMux::handles(const std::string &url) {
	return LibavMux::canHandle(url);
}

bool LibavMux::canHandle(const std::string &/*url*/) {
	return true; //TODO
}

}
