#pragma once

struct mp42tsXOptions {
	std::string url;
};

mp42tsXOptions processArgs(int argc, char const* argv[]);
