#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Out {

class File : public ModuleS {
public:
	File(std::string const& path);
	~File();
	void process(std::shared_ptr<const Data> data) override;

private:
	FILE *file;
};

}
}
