#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"
#include "../common/pcm.hpp"

struct AVCodecContext;

namespace ffpp {
class Frame;
}

class AudioConverter;

namespace Modules {
namespace Decode {

class LibavDecode : public ModuleS {
public:
	LibavDecode(const MetadataPktLibav &metadata);
	~LibavDecode();
	void process(Data data) override;
	void flush() override;

private:
	bool processAudio(const DataAVPacket*);
	bool processVideo(const DataAVPacket*);

	void setTimestamp(std::shared_ptr<DataBase> s, uint64_t increment = 1) const;

	AVCodecContext * const codecCtx;
	std::unique_ptr<ffpp::Frame> const avFrame;
	OutputPicture* videoOutput;
	OutputPcm* audioOutput;
	uint64_t m_numFrames;
};

}
}
