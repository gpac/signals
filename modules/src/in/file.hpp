#pragma once

#include "internal/core/module.hpp"
#include <string>

namespace Modules {

namespace In {

class File : public Module {
public:
	static File* create(std::string const& path);
	~File();
	void process(std::shared_ptr<Data> data);

private:
	File(FILE *file);

	FILE *file;
};

}

}
