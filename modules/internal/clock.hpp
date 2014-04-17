#pragma once

#include "config.hpp"
#include <stdint.h>

namespace Modules {

struct IClock {
	static auto const Rate = 180000LL;
	virtual uint64_t now() const = 0;
};

IClock* createSystemClock(); // move elsewhere

extern MODULES_EXPORT IClock* const g_DefaultClock;
}
