#pragma once

#include "internal/core/data.hpp"
#include "internal/core/pin.hpp"
#include "internal/core/props.hpp"
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
	uint8_t* data()  override;
	uint64_t size() const  override;
	AVPacket* getPacket() const;
	void resize(size_t size)  override;

private:
	std::unique_ptr<AVPacket> const pkt;
};

//TODO: remove: created specially to fix the Transform converters relying on libav + awful hack: derives from PcmData
class DataAVFrame : public Data {
public:
	DataAVFrame(size_t size);
	~DataAVFrame();
	uint8_t* data()  override;
	uint64_t size() const  override;
	AVFrame* getFrame() const;
	void resize(size_t size) override;

private:
	AVFrame *frame;
};

class PcmFormat;
class PcmData;
void libavAudioCtxConvert(const PcmFormat *cfg, AVCodecContext *codecCtx);
void libavFrameDataConvert(const PcmData *data, AVFrame *frame);
void libavFrame2pcmConvert(const AVFrame *frame, PcmFormat *cfg);

void buildAVDictionary(const std::string &moduleName, AVDictionary **dict, const char *options, const char *type);

void avLog(void *avcl, int level, const char *fmt, va_list vl);

typedef PinDataDefault<DataAVPacket> PinLibavPacket;

class PinLibavPacketFactory : public PinFactory {
public:
	Pin* createPin(IProps *props = nullptr);
};

typedef PinDataDefault<Picture> PinLibavFrame;

class PinLibavFrameFactory : public PinFactory {
public:
	Pin* createPin(IProps *props = nullptr);
};

}
