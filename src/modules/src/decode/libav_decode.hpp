#pragma once

#include "internal/core/module.hpp"
#include "../common/libav.hpp"
#include "src/common/pcm.hpp"

struct AVCodecContext;

namespace ffpp {
struct Frame;
}

class AudioConverter;

using namespace Modules;

namespace Decode {

class LibavDecode : public Module {
public:
	LibavDecode(const PropsDecoder &props);
	~LibavDecode();
	void process(std::shared_ptr<const Data> data) override;

private:
	void processAudio(const DataAVPacket*);
	void processVideo(const DataAVPacket*);

	void setTimestamp(std::shared_ptr<Data> s, uint64_t increment = 1) const;

	AVCodecContext * const codecCtx;
	std::unique_ptr<ffpp::Frame> const avFrame;
	PinPicture* videoPin;
	PinPcm* audioPin;
	uint64_t m_numFrames;
};

}
