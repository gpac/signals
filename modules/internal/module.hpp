#pragma once

#include <string>


class IModule {
public:
	virtual bool process() = 0; //Romain
	virtual bool handles(const std::string &url) = 0;
};
