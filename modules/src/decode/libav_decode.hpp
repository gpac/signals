#pragma once

#include "internal/core/module.hpp"
#include "../common/libav.hpp"
#include <string>

struct AVCodecContext;

namespace ffpp {
struct Frame;
}

class AudioConverter;
class VideoConverter;

using namespace Modules;

namespace Decode {

class LibavDecode : public Module {
public:
	static LibavDecode* create(const PropsDecoder &props);
	~LibavDecode();
	void process(std::shared_ptr<Data> data);

private:
	LibavDecode(AVCodecContext *codecCtx);
	void processAudio(DataAVPacket*);
	void processVideo(DataAVPacket*);

	void setTimestamp(std::shared_ptr<Data> s) const;

	std::unique_ptr<AVCodecContext> const codecCtx;
	std::unique_ptr<ffpp::Frame> const avFrame;

	std::unique_ptr<AudioConverter> m_pAudioConverter;
	std::unique_ptr<VideoConverter> m_pVideoConverter;

	uint64_t m_numFrames;
};

}
