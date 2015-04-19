#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"
#include "../common/picture.hpp"

struct AVStream;

namespace ffpp {
class Frame;
}

using namespace Modules;

namespace Encode {

struct LibavEncodeParams {
	//video only
	Resolution res = VIDEO_RESOLUTION;
	int bitrate_v = 300000;
	int GOPSize = 25;
	int frameRate = 25;
	bool isLowLatency = false;

	//audio only
	int bitrate_a = 128000;
};

class LibavEncode : public ModuleS {
public:
	enum Type {
		Video,
		Audio,
		Unknown
	};

	LibavEncode(Type type, const LibavEncodeParams &params = *uptr(new LibavEncodeParams));
	~LibavEncode();
	void process(Data data) override;
	void flush() override;

private:
	bool processAudio(const DataPcm *data);
	bool processVideo(const DataPicture *data);

	AVCodecContext *codecCtx;
	std::unique_ptr<PcmFormat> pcmFormat;
	std::unique_ptr<ffpp::Frame> const avFrame;
	int frameNum;
	OutputDataDefault<DataAVPacket>* output;
};

}
