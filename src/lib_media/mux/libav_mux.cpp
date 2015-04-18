#include "libav_mux.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
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

LibavMux::LibavMux(const std::string &baseName)
: m_headerWritten(false) {
	/* parse the format optionsDict */
	std::string optionsStr = "-format mp4"; //TODO
	AVDictionary *optionsDict = nullptr;
	buildAVDictionary("[libav_mux]", &optionsDict, optionsStr.c_str(), "format");

	/* setup container */
	AVOutputFormat *of = av_guess_format(av_dict_get(optionsDict, "format", nullptr, 0)->value, nullptr, nullptr);
	if (!of) {
		Log::msg(Log::Warning, "[libav_mux] couldn't guess container from file extension");
		av_dict_free(&optionsDict);
		throw std::runtime_error("Container format guess failed");
	}
	av_dict_free(&optionsDict);

	/* output format context */
	m_formatCtx = avformat_alloc_context();
	if (!m_formatCtx) {
		Log::msg(Log::Warning, "[libav_mux] format context couldn't be allocated.");
		throw std::runtime_error("Format Context allocation failed");
	}
	m_formatCtx->oformat = of;

	std::stringstream fileName;
	fileName << baseName;
	std::stringstream formatExts(of->extensions); //get the first extension recommended by ffmpeg
	std::string fileNameExt;
	std::getline(formatExts, fileNameExt, ',');
	fileName << "." << fileNameExt;

	/* open the output file, if needed */
	if (!(m_formatCtx->flags & AVFMT_NOFILE)) {
		if (avio_open(&m_formatCtx->pb, fileName.str().c_str(), AVIO_FLAG_READ_WRITE) < 0) {
			Log::msg(Log::Warning, "[libav_mux] could not open %s, disable output.", baseName);
			avformat_free_context(m_formatCtx);
			throw std::runtime_error("Output open failed");
		}
		strncpy(m_formatCtx->filename, fileName.str().c_str(), sizeof(m_formatCtx->filename));
	}
}

LibavMux::~LibavMux() {
	if (m_formatCtx) {
		av_write_trailer(m_formatCtx); //write the trailer if any
	}
	if (m_formatCtx && !(m_formatCtx->flags & AVFMT_NOFILE)) {
		avio_close(m_formatCtx->pb); //close output file
	}
	if (m_formatCtx) {
		avformat_free_context(m_formatCtx);
	}
}

void LibavMux::declareStream(Data data) {
	auto const metadata_ = data->getMetadata();
	if (auto metadata = std::dynamic_pointer_cast<const MetadataPktLibavVideo>(metadata_)) {
		AVStream *avStream = avformat_new_stream(m_formatCtx, metadata->getAVCodecContext()->codec);
		if (!avStream) {
			Log::msg(Log::Warning, "[LibavMux] could not create the stream, disable output.");
			throw std::runtime_error("[LibavMux] Stream creation failed.");
		}
		if (metadata->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_formatCtx->streams[0]->codec->time_base = metadata->getAVCodecContext()->time_base; //FIXME: [0]: not a mux yet...
			m_formatCtx->streams[0]->codec->width = metadata->getAVCodecContext()->width;
			m_formatCtx->streams[0]->codec->height = metadata->getAVCodecContext()->height;
		}
		if (m_formatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
			m_formatCtx->streams[0]->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
	} else {
		throw std::runtime_error("[LibavMux] Stream creation failed: unknown type.");
	}
}

void LibavMux::ensureHeader() {
	if (!m_headerWritten) {
		if (avformat_write_header(m_formatCtx, nullptr) != 0) {
			Log::msg(Log::Warning, "[libav_mux] fatal error: can't write the container header");
			for (unsigned i = 0; i < m_formatCtx->nb_streams; i++) {
				if (m_formatCtx->streams[i]->codec && m_formatCtx->streams[i]->codec->codec) {
					Log::msg(Log::Debug, "[libav_mux] codec[%s] is \"%s\" (%s)", i, m_formatCtx->streams[i]->codec->codec->name, m_formatCtx->streams[i]->codec->codec->long_name);
				}
			}
		} else {
			m_headerWritten = true;
		}
	}
}

void LibavMux::process(bool dataTypeUpdated) {
	Data data = inputs[0]->pop();
	if (dataTypeUpdated)
		declareStream(data);
	auto encoderData = safe_cast<const DataAVPacket>(data);
	auto pkt = encoderData->getPacket();

	ensureHeader();

	/* Timestamps */
	assert(pkt->pts != (int64_t)AV_NOPTS_VALUE);
	AVStream *avStream = m_formatCtx->streams[0]; //FIXME: fixed '0' for stream num: this is not a mux yet ;)
	pkt->dts = av_rescale_q(pkt->dts, avStream->codec->time_base, avStream->time_base);
	pkt->pts = av_rescale_q(pkt->pts, avStream->codec->time_base, avStream->time_base);
	pkt->duration = (int)av_rescale_q(pkt->duration, avStream->codec->time_base, avStream->time_base);

	/* write the compressed frame to the container output file */
	pkt->stream_index = avStream->index;
	if (av_interleaved_write_frame(m_formatCtx, pkt) != 0) {
		Log::msg(Log::Warning, "[libav_mux] can't write video frame.");
		return;
	}
}

}
