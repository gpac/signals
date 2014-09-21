#pragma once

#include "internal/module.hpp"
#include <string>

namespace Modules {

namespace In {

class File : public Module {
public:
	static File* create(std::string const& path);
	~File();
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);
	bool process(std::shared_ptr<Data> data);

private:
	File(FILE *file);

	FILE *file;
};

}

}
