#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {

namespace In {

class File : public ModuleS {
public:
	File(std::string const& fn);
	~File();
	void process(Data data) override;

private:
	FILE *file;
	OutputDefault* output;
};

}

}
