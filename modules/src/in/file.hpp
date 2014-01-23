#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

namespace Modules {

namespace In {

class MODULES_EXPORT File : public ModuleSync {
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
