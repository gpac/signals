#include "libav_encode.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include <cassert>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

namespace {
void fps2NumDen(const double fps, int &num, int &den) {
	if (fabs(fps - (int)fps) < 0.001) {
		//infer integer frame rates
		num = (int)fps;
		den = 1;
	} else if (fabs((fps*1001.0) / 1000.0 - (int)(fps + 1)) < 0.001) {
		//infer ATSC frame rates
		num = (int)(fps + 1) * 1000;
		den = 1001;
	} else if (fabs(fps * 2 - (int)(fps * 2)) < 0.001) {
		//infer rational frame rates; den = 2
		num = (int)(fps * 2);
		den = 2;
	} else if (fabs(fps * 4 - (int)(fps * 4)) < 0.001) {
		//infer rational frame rates; den = 4
		num = (int)(fps * 4);
		den = 4;
	} else {
		num = (int)fps;
		den = 1;
		Log::msg(Log::Warning, "[libav_encode] Frame rate '%lf' was not recognized. Truncating to '%d'.", fps, num);
	}
}

auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);
}

namespace Encode {

LibavEncode* LibavEncode::create(Type type) {
	return new LibavEncode(type);
}

LibavEncode::LibavEncode(Type type)
	: frameNum(-1) {
	std::string codecOptions, generalOptions, codecType;
	switch (type) {
	case Video:
		codecOptions = "-b 500000 -g 10 -keyint_min 10 -bf 0"; //TODO
		generalOptions = "-vcodec mpeg2video -r 25 -pass 1"; //TODO //Romain: test
		codecType = "vcodec";
		break;
	case Audio:
		codecOptions = "-b 192000"; //TODO
		generalOptions = "-acodec libvo_aacenc"; //TODO
		codecType = "acodec";
		break;
	default:
		throw std::runtime_error("Unknown encoder type. Failed.");
	}

	/* parse the codec optionsDict */
	AVDictionary *codecDict = NULL;
	buildAVDictionary("[libav_encode]", &codecDict, codecOptions.c_str(), "codec");
	av_dict_set(&codecDict, "threads", "auto", 0);

	/* parse other optionsDict*/
	AVDictionary *generalDict = NULL;
	buildAVDictionary("[libav_encode]", &generalDict, generalOptions.c_str(), "other");

	/* find the encoder */
	AVCodec *codec = avcodec_find_encoder_by_name(av_dict_get(generalDict, codecType.c_str(), NULL, 0)->value);
	if (!codec) {
		Log::msg(Log::Warning, "[libav_encode] codec not found, disable output.");
		av_dict_free(&generalDict);
		av_dict_free(&codecDict);
		throw std::runtime_error("Codec not found.");
	}

	codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx) {
		Log::msg(Log::Warning, "[libav_encode] could not allocate the codec context.");
		av_dict_free(&generalDict);
		av_dict_free(&codecDict);
		throw std::runtime_error("Codec context allocation failed.");
	}

	/* parameters */
	int linesize[8];
	switch (type) {
	case Video:
	{
					const int width = 1280; //TODO
					const int height = 720; //TODO
					codecCtx->width = width;
					codecCtx->height = height;
					linesize[0] = codecCtx->width;
					linesize[1] = codecCtx->width / 2;
					linesize[2] = codecCtx->width / 2;
					if (strcmp(av_dict_get(generalDict, "vcodec", NULL, 0)->value, "mjpeg")) {
						codecCtx->pix_fmt = PIX_FMT_YUV420P;
					} else {
						codecCtx->pix_fmt = PIX_FMT_YUVJ420P;
					}

					/* set other optionsDict*/
#if 0 //TODO
					if (avCodec == "h264") {
						av_opt_set(codecCtx->priv_data, "preset", "superfast", 0);
						av_opt_set(codecCtx->priv_data, "rc-lookahead", "0", 0);
					}
					codecCtx->flags |= CODEC_FLAG_PASS1;
					if (atoi(av_dict_get(generalDict, "pass", NULL, 0)->value) == 2) {
						codecCtx->flags |= CODEC_FLAG_PASS2;
					}
#endif
					double fr = atof(av_dict_get(generalDict, "r", NULL, 0)->value);
					AVRational fps;
					fps2NumDen(fr, fps.den, fps.num); //for FPS, num and den are inverted
					codecCtx->time_base = fps;
	}
		break;
	case Audio:
		codecType = "audio";
		codecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
		codecCtx->sample_rate = 44100;
		codecCtx->channels = 2;
		codecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
		break;
	default:
		assert(0);
	}

#if 0 //TODO
	/* user extra params */
	std::string extraParams;
	if (Parse::populateString("LibavOutputWriter", config, "extra_params", extraParams, false) == Parse::PopulateResult_Ok) {
		Log::msg(Log::Debug, "[libav_encode] extra_params : " << extraParams.c_str());
		std::vector<std::string> paramList;
		Util::split(extraParams.c_str(), ',', &paramList);
		auto param = paramList.begin();
		for (; param != paramList.end(); ++param) {
			std::vector<std::string> paramValue;
			Util::split(param->c_str(), '=', &paramValue);
			if (paramValue.size() != 2) {
				Log::msg(Log::Warning, "[libav_encode] extra_params :   wrong param (" << paramValue.size() << " value detected, 2 expected) in " << param->c_str());
			} else {
				Log::msg(Log::Debug, "[libav_encode] extra_params :   detected param " << paramValue[0].c_str() << " with value " << paramValue[1].c_str() << " [" << param->c_str() << "]");
				av_dict_set(&codecDict, paramValue[0].c_str(), paramValue[1].c_str(), 0);
			}
		}
	}
#endif

	av_dict_free(&generalDict);

	/* open it */
	if (avcodec_open2(codecCtx, codec, &codecDict) < 0) {
		Log::msg(Log::Warning, "[libav_encode] could not open codec, disable output.");
		av_dict_free(&codecDict);
		throw std::runtime_error("Codec creation failed.");
	}

	/* check all optionsDict have been consumed */
	AVDictionaryEntry *avde = NULL;
	auto opt = string_dup(codecOptions.c_str());
	char *tok = strtok(opt.data(), "- ");
	while (tok && strtok(NULL, "- ")) {
		if ((avde = av_dict_get(codecDict, tok, avde, 0))) {
			Log::msg(Log::Warning, "[libav_encode] codec option \"%s\", value \"%s\" was ignored.", avde->key, avde->value);
		}
		tok = strtok(NULL, "- ");
	}
	av_dict_free(&codecDict);

	avFrame = avcodec_alloc_frame();
	if (!avFrame) {
		Log::msg(Log::Warning, "[libav_encode] could not create the AVFrame, disable output.");
		avcodec_close(codecCtx);
		throw std::runtime_error("Frame allocation failed.");
	}

	/* AVFrame parameters */
	switch (type) {
	case Video:
		avFrame->linesize[0] = linesize[0];
		avFrame->linesize[1] = linesize[1];
		avFrame->linesize[2] = linesize[2];
		break;
	case Audio:
		avFrame->sample_rate = codecCtx->sample_rate;
		avFrame->nb_samples = codecCtx->frame_size;
		avFrame->channel_layout = codecCtx->channel_layout;
		break;
	default:
		assert(0);
	}

	signals.push_back(pinFactory->createPin());
}

LibavEncode::~LibavEncode() {
	if (codecCtx) {
		avcodec_close(codecCtx);
	}
	if (avFrame) {
		avcodec_free_frame(&avFrame);
	}
}

void LibavEncode::sendOutputPinsInfo() {
	std::shared_ptr<StreamVideo> videoStream(new StreamVideo);
	videoStream->width = codecCtx->width;
	videoStream->height = codecCtx->height;
	videoStream->timeScale = codecCtx->time_base.num / codecCtx->time_base.den;
	videoStream->extradata = codecCtx->extradata;
	videoStream->extradataSize = codecCtx->extradata_size;
	videoStream->codecCtx = codecCtx; //FIXME: all the information above is redundant with this one
	declareStream.emit(videoStream);
}

bool LibavEncode::processAudio(std::shared_ptr<Data> data) {
	std::shared_ptr<DataAVPacket> out(new DataAVPacket);
	AVPacket *pkt = out->getPacket();

	//FIXME: audio are only 2 planes right now...
	avFrame->data[0] = (uint8_t*)data->data();
	avFrame->data[1] = (uint8_t*)data->data() + data->size() / 2;
	avFrame->linesize[0] = (int)data->size() / 2;
	avFrame->linesize[1] = (int)data->size() / 2;
	avFrame->pts = ++frameNum;
	int gotPkt = 0;
	if (avcodec_encode_audio2(codecCtx, pkt, avFrame, &gotPkt)) {
		Log::msg(Log::Warning, "[libav_encode] error encountered while encoding audio frame %d.", frameNum);
		return false;
	} else {
		if (gotPkt) {
			pkt->pts = pkt->dts = avFrame->pts * pkt->duration;
			assert(pkt->size);
			signals[0]->emit(out);
		}
	}

	return true;
}

bool LibavEncode::processVideo(std::shared_ptr<Data> data) {
	std::shared_ptr<DataAVPacket> out(new DataAVPacket);
	AVPacket *pkt = out->getPacket();

	avFrame->data[0] = (uint8_t*)data->data();
	avFrame->data[1] = avFrame->data[0] + codecCtx->width * codecCtx->height;
	avFrame->data[2] = avFrame->data[1] + (codecCtx->width / 2) * (codecCtx->height / 2);
	avFrame->pts = ++frameNum;
	int gotPkt = 0;
	if (avcodec_encode_video2(codecCtx, pkt, avFrame, &gotPkt)) {
		Log::msg(Log::Warning, "[libav_encode] error encountered while encoding video frame %d.", frameNum);
		return false;
	} else {
		if (gotPkt) {
			assert(pkt->size);
			signals[0]->emit(out);
		}
	}

	return true;
}

bool LibavEncode::process(std::shared_ptr<Data> data) {
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

bool LibavEncode::handles(const std::string &url) {
	return LibavEncode::canHandle(url);
}

bool LibavEncode::canHandle(const std::string &/*url*/) {
	return true; //TODO
}

}
