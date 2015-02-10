#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {

namespace In {

class File : public Module {
public:
	static File* create(std::string const& path);
	~File();
	void process(std::shared_ptr<const Data> data) override;

private:
	File(FILE *file);

	FILE *file;
	PinDefault* output;
};

}

}
