#include "libavcodec_55.hpp"
#include "../utils/log.hpp"
#include <cassert>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

namespace Decode {

Libavcodec_55* Libavcodec_55::create(const PropsDecoder &props) {
	struct AVCodecContext *codecCtx = props.getAVCodecContext();
	struct AVCodec *codec = NULL;
	struct AVFrame *frame = NULL;

	avcodec_register_all();
	av_register_all();
	//TODO: custom log: av_log_set_callback(avlog);

	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
	case AVMEDIA_TYPE_AUDIO:
		break;
	default:
		Log::msg(Log::Warning, "Module Libavcodec_55: codec_type not supported. Must be audio or video.");
		return NULL;
	}

	//find an appropriate decoder
	codec = avcodec_find_decoder(codecCtx->codec_id);
	if (!codec) {
		Log::msg(Log::Warning, "Module Libavcodec_55: Codec not found");
		return NULL;
	}

	//TODO: test: force single threaded as h264 probing seems to miss SPS/PPS and seek fails silently
	AVDictionary *th_opt = NULL;
	av_dict_set(&th_opt, "threads", "1", 0);

	//open the codec
	if (avcodec_open2(codecCtx, codec, &th_opt) < 0) {
		Log::msg(Log::Warning, "Module Libavcodec_55: Couldn't open stream");
		return NULL;
	}

	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		//check colorspace
		if ((codecCtx->pix_fmt != PIX_FMT_YUV420P) && (codecCtx->pix_fmt != PIX_FMT_YUVJ420P)) {
			const char *codecName = codecCtx->codec_name ? codecCtx->codec_name : "[unknown]";
			Log::msg(Log::Warning, "Module Libavcodec_55: Unsupported colorspace for codec \"%s\". Only planar YUV 4:2:0 is supported.", codecName);
			return NULL;
		}
		break;
	case AVMEDIA_TYPE_AUDIO:
		break;
	default:
		assert(0);
		return NULL;
	}

	if (!(frame = avcodec_alloc_frame())) {
		Log::msg(Log::Warning, "Module Libavcodec_55: Can't allocate frame");
		avcodec_close(codecCtx);
		return NULL;
	}

	av_dict_free(&th_opt);

	return new Libavcodec_55(codecCtx, frame);
}

Libavcodec_55::Libavcodec_55(AVCodecContext *codecCtx, AVFrame *frame)
: codecCtx(codecCtx), frame(frame) {
	signals.push_back(new Pin<>());
}

Libavcodec_55::~Libavcodec_55() {
	av_free(frame);
	avcodec_close(codecCtx);
	delete signals[0];
}

bool Libavcodec_55::processAudio(std::shared_ptr<Data> data) {
	//TODO
	return true;
}

bool Libavcodec_55::processVideo(std::shared_ptr<Data> data) {
	DataDecoder *decoderData = dynamic_cast<DataDecoder*>(data.get());
	if (!decoderData) {
		return false;
	}
	AVPacket *pkt = decoderData->getPacket();
	int got_picture;
	if (avcodec_decode_video2(codecCtx, frame, &got_picture, pkt) < 0) {
		Log::msg(Log::Warning, "[Libavcodec_55] Error encoutered while decoding.");
		return true;
	}
	if (got_picture) {
		const int frameSize = (codecCtx->width * codecCtx->height * 3) / 2;
		std::shared_ptr<Data> out(new Data(frameSize));
		//TODO: YUV specific + wrap the frame output size
		for (int h = 0; h < codecCtx->height; ++h) {
			memcpy(out.get()->data() + h*codecCtx->width, frame->data[0] + h*frame->linesize[0], codecCtx->width);
		}
		uint8_t *UPlane = out.get()->data() + codecCtx->width * codecCtx->height;
		for (int h = 0; h < codecCtx->height / 2; ++h) {
			memcpy((void*)(UPlane + h*codecCtx->width / 2), frame->data[1] + h*frame->linesize[1], codecCtx->width / 2);
		}
		uint8_t *VPlane = out.get()->data() + (codecCtx->width * codecCtx->height * 5) / 4;
		for (int h = 0; h < codecCtx->height / 2; ++h) {
			memcpy((void*)(VPlane + h*codecCtx->width / 2), frame->data[2] + h*frame->linesize[2], codecCtx->width / 2);
		}
		signals[0]->emit(out);
	}
	return true;
}

bool Libavcodec_55::process(std::shared_ptr<Data> data) {
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

bool Libavcodec_55::handles(const std::string &url) {
	return Libavcodec_55::canHandle(url);
}

bool Libavcodec_55::canHandle(const std::string &url) {
	return true; //TODO
}

}
