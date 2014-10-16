#pragma once

#include "internal/core/module.hpp"


using namespace Modules;

namespace Decode {

class JPEGTurbo;

class JPEGTurboDecode : public Module {
public:
	JPEGTurboDecode();
	~JPEGTurboDecode();
	void process(std::shared_ptr<Data> data);

private:
	std::unique_ptr<JPEGTurbo> const jtHandle;
};

}
