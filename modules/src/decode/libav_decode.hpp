#pragma once

#include "internal/core/module.hpp"
#include "../common/libav.hpp"
#include <string>

struct AVCodecContext;

namespace ffpp {
struct Frame;
}

class AudioConverter;

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

	uint64_t m_numFrames;
};

}
