#pragma once

#include "internal/core/module.hpp"

namespace Modules {
namespace Out {

/**
 * Open bar output. Thread-safe by design ©
 */
class Null : public Module {
public:
	Null();
	void process(std::shared_ptr<Data> data);
};

}
}
