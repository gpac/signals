#include "libav_mux.hpp"
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

namespace Mux {

LibavMux* LibavMux::create(const std::string &baseName) {
	AVFormatContext *formatCtx = NULL;

	/* parse the format optionsDict */
	std::string optionsStr = "-format mp4"; //TODO
	AVDictionary *optionsDict = NULL;
	buildAVDictionary("[libav_mux]", &optionsDict, optionsStr.c_str(), "format");

	/* setup container */
	AVOutputFormat *of = av_guess_format(av_dict_get(optionsDict, "format", NULL, 0)->value, NULL, NULL);
	if (!of) {
		Log::msg(Log::Warning, "[libav_mux] couldn't guess container from file extension");
		av_dict_free(&optionsDict);
		throw std::runtime_error("Container format guess failed");
	}
	av_dict_free(&optionsDict);

	/* output format context */
	formatCtx = avformat_alloc_context();
	if (!formatCtx) {
		Log::msg(Log::Warning, "[libav_mux] format context couldn't be allocated.");
		throw std::runtime_error("Format Context allocation failed");
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
			throw std::runtime_error("Output open failed");
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

void LibavMux::declareStream(std::shared_ptr<Stream> stream_) {
	auto stream = std::dynamic_pointer_cast<StreamVideo>(stream_);
	if (!stream) {
		Log::msg(Log::Warning, "[GPACMuxMP4] Invalid stream declared.");
		return;
	}
	AVStream *avStream = avformat_new_stream(formatCtx, stream->codecCtx->codec);
	if (!avStream) {
		Log::msg(Log::Warning, "[libav_encode] could not create the stream, disable output.");
		throw std::runtime_error("Stream creation failed.");
	}
	if (stream->codecCtx->codec_type == AVMEDIA_TYPE_VIDEO) {
		formatCtx->streams[0]->codec->time_base = stream->codecCtx->time_base; //FIXME: [0]: not a mux yet...
		formatCtx->streams[0]->codec->width = stream->codecCtx->width;
		formatCtx->streams[0]->codec->height = stream->codecCtx->height;
	}
	if (formatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
		formatCtx->streams[0]->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
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
		} else {
			headerWritten = true;
		}
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
