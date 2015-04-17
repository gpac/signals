#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/pcm.hpp"


namespace ffpp {
class SwResampler;
class Frame;
}

namespace Modules {
namespace Transform {

class AudioConvert : public ModuleS {
public:
	AudioConvert(const PcmFormat &dstFormat);
	AudioConvert(const PcmFormat &srcFormat, const PcmFormat &dstFormat);
	~AudioConvert();
	void process(Data data) override;
	void flush() override;

private:
	void configure(const PcmFormat &srcFormat);
	void reconfigure(const PcmFormat &srcFormat);
	PcmFormat srcPcmFormat, dstPcmFormat;
	std::unique_ptr<ffpp::SwResampler> m_Swr;
	uint64_t accumulatedTimeInDstSR;
	OutputPcm* output;
	bool autoConfigure;
};

}
}
