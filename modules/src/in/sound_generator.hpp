#pragma once

#include "internal/core/module.hpp"
#include "../common/pcm.hpp"

namespace Modules {

namespace In {

class SoundGenerator : public Module {
public:
	SoundGenerator();
	void process(std::shared_ptr<const Data> data) override;

private:
	double nextSample();
	uint64_t m_numSamples;
	PcmFormat pcmFormat;
	PinPcm* output;
};

}

}
