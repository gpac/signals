#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>
#include <string>


namespace Modules {

//TODO: separate sync and async?
class EXPORT IModule {
public:
	virtual bool process(std::shared_ptr<Data> data) = 0;
	virtual bool handles(const std::string &url) = 0;
	virtual void destroy() = 0; /* required for async, otherwise we still have callback/futures on an object being destroyed */
};

//specialization
class EXPORT Module {
public:
	virtual bool process(std::shared_ptr<Data> data) = 0;
	virtual bool handles(const std::string &url) = 0;

	/**
	 * Must be called before the destructor.
	 */
	virtual void destroy() {
		for (auto &signal : signals) {
			signal->destroy();
		}
	}

	std::vector<Pin<>*> signals; //TODO: evaluate how public this needs to be
};

}
