#pragma once

struct dashcastXOptions {
	uint64_t segmentDuration;
};

dashcastXOptions processArgs(int argc, char const* argv[]);
