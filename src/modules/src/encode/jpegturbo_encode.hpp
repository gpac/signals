#pragma once

#include "internal/core/module.hpp"

#define JPEG_DEFAULT_QUALITY 70

using namespace Modules;

namespace Encode {

class JPEGTurbo;

class JPEGTurboEncode : public Module {
public:
	JPEGTurboEncode(Resolution resolution, int JPEGQuality = JPEG_DEFAULT_QUALITY);
	~JPEGTurboEncode();
	void process(std::shared_ptr<const Data> data) override;

private:
	PinDefault* output;
	std::unique_ptr<JPEGTurbo> const jtHandle;
	int JPEGQuality;
	Resolution resolution;
};

}
