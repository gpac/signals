#pragma once

#include <stdint.h>

namespace Modules {

struct IClock {
	virtual uint64_t now() const = 0;
};

IClock* createSystemClock(); // move elsewhere
}
