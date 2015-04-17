#pragma once

#include "clock.hpp"

#include <cstdint>
#include <vector>

namespace Modules {

/**
 * A generic timed data container.
 */
class Data {
public:
	virtual ~Data() {
	}
	void setTime(uint64_t timeIn180k) {
		m_TimeIn180k = timeIn180k;
	}
	void setTime(uint64_t timeIn180k, uint64_t timescale) {
		m_TimeIn180k = timescaleToClock(timeIn180k, timescale);
	}
	uint64_t getTime() const {
		return m_TimeIn180k;
	}
	void setDuration(uint64_t DurationIn180k) {
		m_DurationIn180k = DurationIn180k;
	}
	void setDuration(uint64_t DurationInTimescale, uint64_t timescale) {
		m_DurationIn180k = timescaleToClock(DurationInTimescale, timescale);
	}
	uint64_t getDuration() const {
		return m_DurationIn180k;
	}

private:
	uint64_t m_TimeIn180k;
	uint64_t m_DurationIn180k;
};

class RawData : public Data {
public:
	RawData(size_t size) : buffer(size) {
	}
	uint8_t* data() {
		return buffer.data();
	}
	const uint8_t* data() const {
		return buffer.data();
	}
	uint64_t size() const {
		return buffer.size();
	}
	void resize(size_t size) {
		buffer.resize(size);
	}

private:
	std::vector<uint8_t> buffer;
};

}
