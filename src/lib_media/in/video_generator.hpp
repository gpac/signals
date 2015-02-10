#pragma once

#include "lib_modules/core/clock.hpp"
#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"

namespace Modules {

namespace In {

class VideoGenerator : public Module {
public:
	VideoGenerator();
	void process(std::shared_ptr<const Data> data) override;

private:
	uint64_t m_numFrames;
	PinPicture* output;
};

}

}
