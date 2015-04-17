#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"

struct AVStream;

namespace ffpp {
class Frame;
}

using namespace Modules;

namespace Encode {

class LibavEncode : public Module {
public:
	enum Type {
		Video,
		Audio,
		Unknown
	};

	LibavEncode(Type type, bool isLowLatency = false);
	~LibavEncode();
	void process(std::shared_ptr<const Data> data) override;
	void flush() override;

private:
	bool processAudio(const PcmData *data);
	bool processVideo(const Picture *data);

	AVCodecContext *codecCtx;
	std::unique_ptr<PcmFormat> pcmFormat;
	std::unique_ptr<ffpp::Frame> const avFrame;
	int frameNum;
	bool isLowLatency;
	PinDataDefault<DataAVPacket>* output;
};

}
