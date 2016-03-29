#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/pcm.hpp"

namespace Modules {

namespace In {

class SoundGenerator : public ModuleS {
	public:
		SoundGenerator();
		void process(Data data) override;

	private:
		double nextSample();
		uint64_t m_numSamples;
		PcmFormat pcmFormat;
		OutputPcm* output;
};

}

}
