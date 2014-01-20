#include "libav_decode.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include <cassert>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

namespace {
auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);
}

namespace Decode {

LibavDecode* LibavDecode::create(const PropsDecoder &props) {
	auto codecCtx = props.getAVCodecContext();

	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
	case AVMEDIA_TYPE_AUDIO:
		break;
	default:
		Log::msg(Log::Warning, "Module LibavDecode: codec_type not supported. Must be audio or video.");
		throw std::runtime_error("Unknown decoder type. Failed.");
	}

	//find an appropriate decoder
	auto codec = avcodec_find_decoder(codecCtx->codec_id);
	if (!codec) {
		Log::msg(Log::Warning, "Module LibavDecode: Codec not found");
		throw std::runtime_error("Decoder not found.");
	}

	//TODO: test: force single threaded as h264 probing seems to miss SPS/PPS and seek fails silently
	AVDictionary *th_opt = NULL;
	av_dict_set(&th_opt, "threads", "1", 0);

	//open the codec
	if (avcodec_open2(codecCtx, codec, &th_opt) < 0) {
		Log::msg(Log::Warning, "Module LibavDecode: Couldn't open stream");
		throw std::runtime_error("Couldn't open stream.");
	}

	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		//check colorspace
		if ((codecCtx->pix_fmt != PIX_FMT_YUV420P) && (codecCtx->pix_fmt != PIX_FMT_YUVJ420P)) {
			const char *codecName = codecCtx->codec_name ? codecCtx->codec_name : "[unknown]";
			Log::msg(Log::Warning, "Module LibavDecode: Unsupported colorspace for codec \"%s\". Only planar YUV 4:2:0 is supported.", codecName);
			throw std::runtime_error("unsupported colorspace.");
		}
		break;
	case AVMEDIA_TYPE_AUDIO:
		break;
	default:
		assert(0);
		throw std::runtime_error("Unknown decoder type. Failed.");
	}

	auto avFrame = avcodec_alloc_frame();
	if (!avFrame) {
		Log::msg(Log::Warning, "Module LibavDecode: Can't allocate frame");
		avcodec_close(codecCtx);
		throw std::runtime_error("Frame allocation failed.");
	}

	av_dict_free(&th_opt);

	return new LibavDecode(codecCtx, avFrame);
}

LibavDecode::LibavDecode(AVCodecContext *codecCtx, AVFrame *avFrame)
: codecCtx(codecCtx), avFrame(avFrame) {
	signals.push_back(uptr(new Pin<>()));
}

LibavDecode::~LibavDecode() {
	av_free(avFrame);
	avcodec_close(codecCtx);
}

bool LibavDecode::processAudio(std::shared_ptr<Data> data) {
	DataAVPacket *decoderData = dynamic_cast<DataAVPacket*>(data.get());
	if (!decoderData) {
		return false;
	}
	AVPacket *pkt = decoderData->getPacket();
	int gotFrame;
	if (avcodec_decode_audio4(codecCtx, avFrame, &gotFrame, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding audio.");
		return true;
	}
	if (gotFrame) {
		const int bufferSize = av_samples_get_buffer_size(nullptr, codecCtx->channels, avFrame->nb_samples, codecCtx->sample_fmt, 0);
		std::shared_ptr<Data> out(new Data(bufferSize));
		if (av_sample_fmt_is_planar(codecCtx->sample_fmt)) {
			size_t index = 0;
			for (int i = 0; i < codecCtx->channels; ++i) {
				const int channelSize = av_samples_get_buffer_size(nullptr, 1, avFrame->nb_samples, codecCtx->sample_fmt, 0);
				memcpy(out->data() + index, avFrame->data[i], channelSize);
				index += channelSize;
			}
		} else {
			memcpy(out->data(), avFrame->data[0], bufferSize);
		}
		signals[0]->emit(out);
	}

	return true;
}

bool LibavDecode::processVideo(std::shared_ptr<Data> data) {
	DataAVPacket *decoderData = dynamic_cast<DataAVPacket*>(data.get());
	if (!decoderData) {
		return false;
	}
	AVPacket *pkt = decoderData->getPacket();
	int gotPicture;
	if (avcodec_decode_video2(codecCtx, avFrame, &gotPicture, pkt) < 0) {
		Log::msg(Log::Warning, "[LibavDecode] Error encoutered while decoding video.");
		return true;
	}
	if (gotPicture) {
		const int frameSize = (codecCtx->width * codecCtx->height * 3) / 2;
		std::shared_ptr<Data> out(new Data(frameSize));
		//TODO: YUV specific + wrap the avFrame output size
		for (int h = 0; h < codecCtx->height; ++h) {
			memcpy(out->data() + h*codecCtx->width, avFrame->data[0] + h*avFrame->linesize[0], codecCtx->width);
		}
		uint8_t *UPlane = out->data() + codecCtx->width * codecCtx->height;
		for (int h = 0; h < codecCtx->height / 2; ++h) {
			memcpy((void*)(UPlane + h*codecCtx->width / 2), avFrame->data[1] + h*avFrame->linesize[1], codecCtx->width / 2);
		}
		uint8_t *VPlane = out->data() + (codecCtx->width * codecCtx->height * 5) / 4;
		for (int h = 0; h < codecCtx->height / 2; ++h) {
			memcpy((void*)(VPlane + h*codecCtx->width / 2), avFrame->data[2] + h*avFrame->linesize[2], codecCtx->width / 2);
		}
		signals[0]->emit(out);
	}
	return true;
}

bool LibavDecode::process(std::shared_ptr<Data> data) {
	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		return processVideo(data);
		break;
	case AVMEDIA_TYPE_AUDIO:
		return processAudio(data);
		break;
	default:
		assert(0);
		return false;
	}
}

bool LibavDecode::handles(const std::string &url) {
	return LibavDecode::canHandle(url);
}

bool LibavDecode::canHandle(const std::string &/*url*/) {
	return true; //TODO
}

}
