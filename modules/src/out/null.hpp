#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

using namespace Modules;

namespace Out {

/**
 * Open bar output. Thread-safe by design ©
 */
class MODULES_EXPORT Null : public Module {
public:
	static Null* create();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	Null();
};

}
