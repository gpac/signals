#pragma once

#include "lib_modules/core/module.hpp"


using namespace Modules;

namespace Decode {

class JPEGTurbo;

class JPEGTurboDecode : public Module {
public:
	JPEGTurboDecode();
	~JPEGTurboDecode();
	void process(std::shared_ptr<const Data> data) override;

private:
	PinDefault* output;
	void ensureMetadata(int width, int height, int pixelFmt);
	std::unique_ptr<JPEGTurbo> const jtHandle;
};

}
