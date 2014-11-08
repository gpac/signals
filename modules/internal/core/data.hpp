#pragma once

#include "clock.hpp"

#include <cstdint>
#include <vector>
#include <stdlib.h>

namespace Modules {

/**
 * A generic data container.
 */
class IData {
public:
	virtual ~IData() {
	}
	virtual uint8_t* data() = 0;
	virtual uint64_t size() const = 0;
	virtual void resize(size_t size) = 0;
};

//FIXME: this class contains multimedia considerations, thus should be in libmm...
class Data : public IData {
public:
	Data(size_t size) : ptr(size) {
	}

	virtual ~Data() {
	}

	uint8_t* data() {
		return ptr.data();
	}

	uint64_t size() const {
		return ptr.size();
	}

	void resize(size_t size) {
		ptr.resize(size);
	}

	void setTime(uint64_t timeIn180k) {
		m_TimeIn180k = timeIn180k;
	}

	uint64_t getTime() const {
		return m_TimeIn180k;
	}

	void setDuration(uint64_t DurationIn180k) {
		m_DurationIn180k = DurationIn180k;
	}

	void setDuration(uint64_t DurationInTimescale, uint64_t timescale) {
		m_DurationIn180k = (DurationInTimescale * IClock::Rate + timescale / 2) / timescale;
	}

	uint64_t getDuration() const {
		return m_DurationIn180k;
	}

private:
	std::vector<uint8_t> ptr;
	uint64_t m_TimeIn180k;
	uint64_t m_DurationIn180k;
};

static const int VIDEO_WIDTH = 1280; //Romain
static const int VIDEO_HEIGHT = 720;
static const int VIDEO_FPS = 24;

static const int AUDIO_SAMPLERATE = 44100;
}
