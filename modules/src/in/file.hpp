#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

namespace Modules {

namespace In {

class MODULES_EXPORT File : public Module {
public:
	static File* create(std::string const& path);
	~File();
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

	void push();

private:
	File(FILE *file);
	bool process(std::shared_ptr<Data> data);

	FILE *file;
};

}

}
