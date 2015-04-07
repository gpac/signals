#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {

namespace In {

class File : public Module {
public:
	File(std::string const& fn);
	~File();
	void process(std::shared_ptr<const Data> data) override;

private:
	FILE *file;
	PinDefault* output;
};

}

}
