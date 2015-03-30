#pragma once

struct dashcastXOptions {
	std::string url;
	uint64_t segmentDuration = 2000;
};

dashcastXOptions processArgs(int argc, char const* argv[]);
