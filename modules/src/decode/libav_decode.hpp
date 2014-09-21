#pragma once

#include "internal/module.hpp"
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
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	LibavDecode(AVCodecContext *codecCtx);
	bool processAudio(DataAVPacket*);
	bool processVideo(DataAVPacket*);

	void setTimestamp(std::shared_ptr<Data> s) const;

	std::unique_ptr<AVCodecContext> const codecCtx;
	std::unique_ptr<ffpp::Frame> const avFrame;

	std::unique_ptr<AudioConverter> m_pAudioConverter;
	std::unique_ptr<VideoConverter> m_pVideoConverter;

	uint64_t m_numFrames;
};

}
