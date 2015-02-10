#include "libav_encode.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "../common/pcm.hpp"
#include <cassert>

#include "lib_ffpp/ffpp.hpp"

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
		Log::msg(Log::Warning, "[libav_encode] Frame rate '%s' was not recognized. Truncating to '%s'.", fps, num);
	}
}

auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);
}

namespace Encode {

LibavEncode::LibavEncode(Type type)
	: pcmFormat(new PcmFormat()), avFrame(new ffpp::Frame), frameNum(-1) {
	std::string codecOptions, generalOptions, codecName;
	switch (type) {
	case Video:
		codecOptions = "-b 500000 -g 10 -keyint_min 10 -bf 0"; //TODO
		generalOptions = "-vcodec libx264 -r 25 -pass 1"; //TODO
		codecName = "vcodec";
		break;
	case Audio:
		codecOptions = "-b 192000"; //TODO
		generalOptions = "-acodec libvo_aacenc"; //TODO
		codecName = "acodec";
		break;
	default:
		throw std::runtime_error("Unknown encoder type. Failed.");
	}

	output = addPin(new PinDataDefault<DataAVPacket>);

	/* parse the codec optionsDict */
	ffpp::Dict codecDict;
	buildAVDictionary("[libav_encode]", &codecDict, codecOptions.c_str(), "codec");
	codecDict.set("threads", "auto");

	/* parse other optionsDict*/
	ffpp::Dict generalDict;
	buildAVDictionary("[libav_encode]", &generalDict, generalOptions.c_str(), "other");

	/* find the encoder */
	auto entry = generalDict.get(codecName);
	if(!entry) {
		throw std::runtime_error("Could not get codecName.");
	}
	AVCodec *codec = avcodec_find_encoder_by_name(entry->value);
	if (!codec) {
		Log::msg(Log::Warning, "[libav_encode] codec '%s' not found, disable output.", entry->value);
		throw std::runtime_error(format("Codec '%s' not found.", entry->value));
	}

	codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx) {
		Log::msg(Log::Warning, "[libav_encode] could not allocate the codec context.");
		throw std::runtime_error("Codec context allocation failed.");
	}

	/* parameters */
	switch (type) {
	case Video: {
		codecCtx->width = VIDEO_RESOLUTION.width; //FIXME: encode size should be a parameter
		codecCtx->height = VIDEO_RESOLUTION.height;
		if (strcmp(generalDict.get("vcodec")->value, "mjpeg")) {
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
		double fr = atof(generalDict.get("r")->value);
		AVRational fps;
		fps2NumDen(fr, fps.den, fps.num); //for FPS, num and den are inverted
		codecCtx->time_base = fps;
	}
	break;
	case Audio:
		libavAudioCtxConvert(pcmFormat.get(), codecCtx);
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

	/* open it */
	codecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER; //gives access to the extradata (e.g. H264 SPS/PPS, etc.)
	if (avcodec_open2(codecCtx, codec, &codecDict) < 0) {
		Log::msg(Log::Warning, "[libav_encode] could not open codec, disable output.");
		throw std::runtime_error("Codec creation failed.");
	}

	/* check all optionsDict have been consumed */
	auto opt = stringDup(codecOptions.c_str());
	char *tok = strtok(opt.data(), "- ");
	while (tok && strtok(NULL, "- ")) {
		AVDictionaryEntry *avde = nullptr;
		avde = codecDict.get(tok, avde);
		if (avde) {
			Log::msg(Log::Warning, "[libav_encode] codec option \"%s\", value \"%s\" was ignored.", avde->key, avde->value);
		}
		tok = strtok(NULL, "- ");
	}
}

void LibavEncode::flush() {
	if (codecCtx && (codecCtx->codec->capabilities & CODEC_CAP_DELAY)) {
		switch (codecCtx->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			while (processVideo(nullptr)) {}
			break;
		case AVMEDIA_TYPE_AUDIO:
			while (processAudio(nullptr)) {}
			break;
		default:
			assert(0);
			break;
		}
	}
}

LibavEncode::~LibavEncode() {
	if (codecCtx) {
		avcodec_close(codecCtx);
	}
}

std::string LibavEncode::getCodecName() const {
	return avcodec_get_name(codecCtx->codec_id);
}

void LibavEncode::sendOutputPinsInfo() {
	assert(pins.size() == 1); //FIXME: tested with 1 output pin only
	if (codecCtx->codec_type == AVMEDIA_TYPE_VIDEO) {
		std::shared_ptr<StreamVideo> stream(new StreamVideo);
		stream->width = codecCtx->width;
		stream->height = codecCtx->height;
		stream->timeScale = codecCtx->time_base.den / codecCtx->time_base.num;
		assert(codecCtx->time_base.num == 1); //FIXME
		stream->extradata = codecCtx->extradata;
		stream->extradataSize = codecCtx->extradata_size;
		stream->codecCtx = codecCtx; //FIXME: all the information above is redundant with this one
		pins[0]->emit(stream);
	} else if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
		std::shared_ptr<StreamAudio> stream(new StreamAudio); //TODO: should use a constructor
		stream->codecName = getCodecName();
		stream->numChannels = codecCtx->channels;
		stream->sampleRate = codecCtx->sample_rate;
		stream->bitsPerSample = av_get_bytes_per_sample(codecCtx->sample_fmt) * 8;
		stream->frameSize = codecCtx->frame_size;
		stream->extradata = codecCtx->extradata;
		stream->extradataSize = codecCtx->extradata_size;
		stream->codecCtx = codecCtx; //FIXME: all the information above is redundant with this one
		pins[0]->emit(stream);
	} else {
		assert(0); //TODO test with anythng else than audio and video
	}
}

bool LibavEncode::processAudio(const PcmData *data) {
	auto out = output->getBuffer(0);
	AVPacket *pkt = out->getPacket();
	AVFrame *f = nullptr;
	if (data) {
		f = avFrame->get();
		libavFrameDataConvert(data, f);
		avFrame->get()->pts = ++frameNum;
	}

	int gotPkt = 0;
	if (avcodec_encode_audio2(codecCtx, pkt, f, &gotPkt)) {
		Log::msg(Log::Warning, "[libav_encode] error encountered while encoding audio frame %s.", frameNum);
		return false;
	}
	if (gotPkt) {
		pkt->pts = pkt->dts = frameNum * pkt->duration;
		out->setDuration(pkt->duration * codecCtx->time_base.num, codecCtx->time_base.den);
		assert(pkt->size);
		pins[0]->emit(out);
		return true;
	}

	return false;
}

bool LibavEncode::processVideo(const Picture *pic) {
	auto out = output->getBuffer(0);
	AVPacket *pkt = out->getPacket();

	std::shared_ptr<ffpp::Frame> f;
	if (pic) {
		f = std::make_shared<ffpp::Frame>();
		f->get()->pict_type = AV_PICTURE_TYPE_NONE;
		f->get()->pts = ++frameNum;
		for (int i = 0; i < 3; ++i) {
			f->get()->width = pic->getResolution().width;
			f->get()->height = pic->getResolution().height;
			f->get()->data[i] = pic->getComp(i);
			f->get()->linesize[i] = (int)pic->getPitch(i);
		}
	}

	int gotPkt = 0;
	if (avcodec_encode_video2(codecCtx, pkt, f ? f->get() : nullptr, &gotPkt)) {
		Log::msg(Log::Warning, "[libav_encode] error encountered while encoding video frame %s.", frameNum);
		return false;
	} else {
		if (gotPkt) {
			assert(pkt->size);
			if (pkt->duration <= 0) {
				pkt->duration = codecCtx->time_base.num;
			}
			out->setDuration(pkt->duration * codecCtx->time_base.num, codecCtx->time_base.den);
			pins[0]->emit(out);
			return true;
		}
	}

	return false;
}

void LibavEncode::process(std::shared_ptr<const Data> data) {
	switch (codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO: {
		const auto encoderData = safe_cast<const Picture>(data);
		processVideo(encoderData.get());
		break;
	}
	case AVMEDIA_TYPE_AUDIO: {
		const auto pcmData = safe_cast<const PcmData>(data);
		if (pcmData->getFormat() != *pcmFormat)
			throw std::runtime_error("[SDLAudio] Incompatible audio data");
		processAudio(pcmData.get());
		break;
	}
	default:
		assert(0);
		return;
	}
}

}
