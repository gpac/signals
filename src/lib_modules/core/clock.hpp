#pragma once

#include <stdint.h>

namespace Modules {

struct IClock {
static auto const Rate = 180000ULL;
virtual uint64_t now() const = 0;
};

IClock* createSystemClock(); // move elsewhere

extern IClock* const g_DefaultClock;

static uint64_t convertToTimescale(uint64_t time, uint64_t timescaleSrc, uint64_t timescaleDst) {
	return (time * timescaleDst + timescaleSrc / 2) / timescaleSrc;
}

static uint64_t timescaleToClock(uint64_t time, uint64_t timescale) {
	return convertToTimescale(time, timescale, IClock::Rate);
}

static uint64_t clockToTimescale(uint64_t time, uint64_t timescale) {
	return convertToTimescale(time, IClock::Rate, timescale);
}
}
