#pragma once

struct dashcastXOptions {
	std::string url;
	uint64_t segmentDuration;
};

dashcastXOptions processArgs(int argc, char const* argv[]);
