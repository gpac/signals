#pragma once

#include "internal/module.hpp"
#include <string>

namespace Modules {

namespace In {

class SoundGenerator : public Module {
public:
	SoundGenerator();
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);
	bool process(std::shared_ptr<Data> data);

private:
	double nextSample();
	uint64_t m_numSamples;
};

}

}
