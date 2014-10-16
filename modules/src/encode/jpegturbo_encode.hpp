#pragma once

#include "internal/core/module.hpp"

#define JPEG_DEFAULT_QUALITY 70

using namespace Modules;

namespace Encode {

class JPEGTurbo;

class JPEGTurboEncode : public Module {
public:
	JPEGTurboEncode(int width, int height, int JPEGQuality = JPEG_DEFAULT_QUALITY);
	~JPEGTurboEncode();
	void process(std::shared_ptr<Data> data);

private:
	std::unique_ptr<JPEGTurbo> const jtHandle;
	int JPEGQuality;
	int width;
	int height;
};

}
