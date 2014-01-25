#pragma once

#include <../modules/modules.hpp>

namespace MM {

/**
 * A synchronous module to be plugged within a Module
 */
class Submodule : public Modules::Module {
public:
	virtual ~Submodule() {
	}
};

}