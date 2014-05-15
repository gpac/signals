#pragma once

#include "internal/clock.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

namespace Modules {

namespace In {

class VideoGenerator : public Module {
public:
	VideoGenerator();
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);
	bool process(std::shared_ptr<Data> data);

private:
	uint64_t m_numFrames;
};

}

}
