#pragma once

#include "lib_media/common/picture.hpp"

struct dashcastXOptions {
	std::string url;
	Modules::Resolution res = Modules::VIDEO_RESOLUTION;
	uint64_t segmentDuration = 2000;
	bool isLive = false;
};

dashcastXOptions processArgs(int argc, char const* argv[]);
