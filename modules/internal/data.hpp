#pragma once

#include "config.hpp"

#include <cstdint>
#include <vector>
#include <stdlib.h>

namespace Modules {

/**
 * A generic data container.
 */
class MODULES_EXPORT IData {
public:
	virtual ~IData() {
	}
	virtual uint8_t* data() = 0;
	virtual uint64_t size() const = 0;
	virtual void resize(size_t size) = 0;
};

class MODULES_EXPORT Data : public IData {
public:
	Data(size_t size) : ptr(size) {
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

private:
	std::vector<uint8_t> ptr;
	uint64_t m_TimeIn180k;
};

class MODULES_EXPORT PcmData : public Data {
public:
	PcmData(size_t size) : Data(size) {
	}
};

static const int VIDEO_WIDTH = 720;
static const int VIDEO_HEIGHT = 576;

static const int AUDIO_SAMPLERATE = 44100;
}
