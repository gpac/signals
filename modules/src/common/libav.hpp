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

//FIXME: move in libmm, or add a new namespace
class PropsDecoder : public IProps {
public:
	/**
	 * Doesn't take the ownership
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
	uint64_t size() const;
	AVPacket* getPacket() const;
	void resize(size_t size);

private:
	std::unique_ptr<AVPacket> const pkt;
};

//TODO: remove: created specially to fix the Transform converters relying on libav
class DataAVFrame : public Data {
public:
	DataAVFrame(size_t size);
	~DataAVFrame();
	uint8_t* data();
	uint64_t size() const;
	AVFrame* getFrame() const;
	void resize(size_t size);

private:
	AVFrame *frame;
};

void buildAVDictionary(const std::string &moduleName, AVDictionary **dict, const char *options, const char *type);

void avLog(void *avcl, int level, const char *fmt, va_list vl);

typedef PinDataDefault<DataAVPacket> PinLibavPacket;

class PinLibavPacketFactory : public PinFactory {
public:
	PinLibavPacketFactory();
	Pin* createPin(IProps *props = nullptr);
};

typedef PinDataDefault<DataAVFrame> PinLibavFrame;

class PinLibavFrameFactory : public PinFactory {
public:
	PinLibavFrameFactory();
	Pin* createPin(IProps *props = nullptr);
};

}
