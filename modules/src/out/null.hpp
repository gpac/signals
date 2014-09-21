#pragma once

#include "internal/module.hpp"
#include <string>

using namespace Modules;

namespace Out {

/**
 * Open bar output. Thread-safe by design ©
 */
class Null : public Module {
public:
	static Null* create();
	bool process(std::shared_ptr<Data> data);

private:
	Null();
};

}
