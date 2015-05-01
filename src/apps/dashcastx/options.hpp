#pragma once

#include "lib_media/common/picture.hpp"

struct dashcastXOptions {
	std::string url;
	std::vector<Modules::Resolution> res;
	uint64_t segmentDuration = 2000;
	bool isLive = false;
};

dashcastXOptions processArgs(int argc, char const* argv[]);
