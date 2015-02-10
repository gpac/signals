#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Out {

/**
 * Open bar output. Thread-safe by design ©
 */
class Null : public Module {
public:
	Null();
	void process(std::shared_ptr<const Data> data) override;
};

}
}
