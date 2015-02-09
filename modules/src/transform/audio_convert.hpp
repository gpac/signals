#pragma once

#include "internal/core/module.hpp"
#include "../common/pcm.hpp"


namespace ffpp {
struct SwResampler;
struct Frame;
}

namespace Modules {
namespace Transform {

class AudioConvert : public Module {
public:
	AudioConvert(PcmFormat srcFormat, PcmFormat dstFormat);
	~AudioConvert();
	void process(std::shared_ptr<const Data> data) override;
	void flush() override;

private:
	PcmFormat srcPcmFormat, dstPcmFormat;
	std::unique_ptr<ffpp::SwResampler> const m_Swr;
	uint64_t accumulatedTimeInDstSR;
	PinPcm* output;
};

}
}
