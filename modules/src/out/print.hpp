#pragma once

#include "../utils/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

using namespace Modules;

class EXPORT Print : public Module {
public:
	static Print* create(std::ostream &os);
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	Print(std::ostream &os);

	std::ostream &os;
};

