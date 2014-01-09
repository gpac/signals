#pragma once

#include "internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>


class EXPORT Print : public IModule {
public:
	static Print* create(const Param &parameters);
	bool process();
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	Print();
};

