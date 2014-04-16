#pragma once

#include "config.hpp"

namespace Modules {
/**
* A generic property container. Help modules communicate with each other.
*/
class MODULES_EXPORT IProps {
public:
	virtual ~IProps() { }
};

}
