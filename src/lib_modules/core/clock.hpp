#pragma once

#include <stdint.h>

namespace Modules {

struct IClock {
static auto const Rate = 180000ULL;
virtual uint64_t now() const = 0;
};

IClock* createSystemClock(); // move elsewhere

extern IClock* const g_DefaultClock;

static uint64_t timescaleToClock(uint64_t time, uint64_t timescale) {
	return (time * IClock::Rate + timescale / 2) / timescale;
}

static uint64_t clockToTimescale(uint64_t time, uint64_t timescale) {
	return (time * timescale + IClock::Rate / 2) / IClock::Rate;
}
}
