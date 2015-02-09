#pragma once

#include "internal/core/clock.hpp"
#include "internal/core/module.hpp"

namespace Modules {

namespace In {

class VideoGenerator : public Module {
public:
	VideoGenerator();
	void process(std::shared_ptr<const Data> data) override;

private:
	uint64_t m_numFrames;
	PinDataDefault<Picture>* output;
};

}

}
