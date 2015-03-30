#pragma once

struct dashcastXOptions {
	std::string url;
	uint64_t segmentDuration = 2000;
	bool isLive = false;
};

dashcastXOptions processArgs(int argc, char const* argv[]);
