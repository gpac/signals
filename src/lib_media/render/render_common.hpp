#pragma once

#include <lib_modules/core/clock.hpp>

namespace Modules {
namespace Render {

static auto const PREROLL_DELAY_IN_MS = 500ULL;
static auto const PREROLL_DELAY = timescaleToClock(PREROLL_DELAY_IN_MS, 1000);

}
}
