#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include <../modules/modules.hpp> //FIXME: for Data, etc.
#include <string>

namespace MM {

class MM_EXPORT File : public Modules::Module { //FIXME: this is a duplicate of Modules::In::File
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
