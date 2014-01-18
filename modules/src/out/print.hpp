#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

using namespace Modules;

namespace Out {

class MODULES_EXPORT Print : public Module {
public:
	static Print* create(std::ostream &os);
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	Print(std::ostream &os);

	std::ostream &os;
};

}
