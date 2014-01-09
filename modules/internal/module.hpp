#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>
#include <string>


class EXPORT IModule {
public:
	virtual bool process(Data *data) = 0;
	virtual bool handles(const std::string &url) = 0;
};

//specialization
class EXPORT Module {
public:
	virtual bool process(Data *data) = 0;
	virtual bool handles(const std::string &url) = 0;

	std::vector<Pin*> signals; //TODO: evaluate how public this needs to be
};
