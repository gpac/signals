#pragma once

#include "lib_modules/core/clock.hpp"
#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"

namespace Modules {

namespace In {

class VideoGenerator : public ModuleS {
public:
	VideoGenerator();
	void process(Data data) override;

private:
	uint64_t m_numFrames;
	OutputPicture* output;
};

}

}
