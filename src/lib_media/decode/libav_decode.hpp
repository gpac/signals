#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"
#include "../common/pcm.hpp"

struct AVCodecContext;

namespace ffpp {
class Frame;
}

class AudioConverter;

using namespace Modules;

namespace Decode {

class LibavDecode : public Module {
public:
	LibavDecode(const PropsDecoder &props);
	~LibavDecode();
	void process(std::shared_ptr<const Data> data) override;
	void flush() override;

private:
	bool processAudio(const DataAVPacket*);
	bool processVideo(const DataAVPacket*);

	void setTimestamp(std::shared_ptr<Data> s, uint64_t increment = 1) const;

	AVCodecContext * const codecCtx;
	std::unique_ptr<ffpp::Frame> const avFrame;
	PinPicture* videoPin;
	PinPcm* audioPin;
	uint64_t m_numFrames;
};

}
