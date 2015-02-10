#pragma once

#include "lib_modules/core/data.hpp"
#include "lib_modules/core/pin.hpp"
#include "lib_modules/core/props.hpp"
#include <cstdarg>
#include <memory>

struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct AVDictionary;

namespace Modules {

class PropsDecoder : public IProps {
public:
	/**
	 * Doesn't take the ownership of codecCtx
	 */
	PropsDecoder(AVCodecContext *codecCtx)
		: codecCtx(codecCtx) {
	}

	AVCodecContext* getAVCodecContext() const {
		return codecCtx;
	}

private:
	AVCodecContext *codecCtx;
};

class DataAVPacket : public Data {
public:
	DataAVPacket(size_t size = 0);
	~DataAVPacket();
	uint8_t* data();
	uint8_t const* data() const;
	uint64_t size() const;
	AVPacket* getPacket() const;
	void resize(size_t size);

private:
	std::unique_ptr<AVPacket> const pkt;
};

class PcmFormat;
class PcmData;
void libavAudioCtxConvertLibav(const PcmFormat *cfg, int &sampleRate, int &format, int &numChannels, uint64_t &layout);
void libavAudioCtxConvert(const PcmFormat *cfg, AVCodecContext *codecCtx);
void libavFrameDataConvert(const PcmData *data, AVFrame *frame);
void libavFrame2pcmConvert(const AVFrame *frame, PcmFormat *cfg);

void buildAVDictionary(const std::string &moduleName, AVDictionary **dict, const char *options, const char *type);

void avLog(void *avcl, int level, const char *fmt, va_list vl);

typedef PinDataDefault<DataAVPacket> PinLibavPacket;
typedef PinDataDefault<Picture> PinPicture;

}
