#pragma once

#include "internal/core/clock.hpp"
#include "internal/core/module.hpp"
#include <string>

namespace Modules {

namespace In {

class VideoGenerator : public Module {
public:
	VideoGenerator();
	void process(std::shared_ptr<Data> data);

private:
	uint64_t m_numFrames;
};

}

}
