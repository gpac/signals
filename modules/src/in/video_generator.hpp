#pragma once

#include "internal/clock.hpp"
#include "internal/module.hpp"
#include <string>

namespace Modules {

namespace In {

class VideoGenerator : public Module {
public:
	VideoGenerator();
	bool process(std::shared_ptr<Data> data);

private:
	uint64_t m_numFrames;
};

}

}
