#pragma once

#include "lib_media/common/picture.hpp"

struct Video {
	Video(Modules::Resolution res, unsigned bitrate) : res(res), bitrate(bitrate) {}
	Modules::Resolution res;
	unsigned bitrate;
};

struct dashcastXOptions {
	std::string url;
	std::vector<Video> v;
	uint64_t segmentDuration = 2000;
	bool isLive = false;
};

dashcastXOptions processArgs(int argc, char const* argv[]);
