#pragma once

#include "clock.hpp"

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>

namespace Modules {

/**
 * A generic data container.
 */
class Data {
public:
	virtual ~Data() {
	}
	virtual uint8_t* data() = 0;
	virtual uint64_t size() const = 0;
	virtual void resize(size_t size) = 0;

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
	uint64_t m_TimeIn180k;
	uint64_t m_DurationIn180k;
};

class RawData : public Data {
public:
	RawData(size_t size) : buffer(size) {
	}

	virtual uint8_t* data() override {
		return buffer.data();
	}

	virtual uint64_t size() const override {
		return buffer.size();
	}

	virtual void resize(size_t size) override {
		buffer.resize(size);
	}

private:
	std::vector<uint8_t> buffer;
};

struct Resolution {
	Resolution() : width(0), height(0) {
	}
	Resolution(int w, int h) : width(w), height(h) {
	}
	bool operator==(Resolution const& other) const {
		return width == other.width && height == other.height;
	}
	bool operator!=(Resolution const& other) const {
		return !(*this == other);
	}
	int width, height;
	size_t yv12size() const {
		return width * height * 3 / 2;
	}
	std::string toString() const {
		std::stringstream ss;
		ss << width << "x" << height;
		return ss.str();
	}
};

class Picture : public RawData {
public:
	Picture(size_t unused) : RawData(0) {
	}
	Picture(Resolution res) : RawData(res.yv12size()) {
		setResolution(res);
	}
	uint8_t* getComp(int comp) const {
		return m_comp[comp];
	}
	size_t getPitch(int comp) const {
		return m_pitch[comp];
	}
	void setResolution(Resolution res) {
		m_res = res;
		resize(res.yv12size());
		auto const numPixels = res.width * res.height;
		m_comp[0] = data();
		m_comp[1] = data() + numPixels;
		m_comp[2] = data() + numPixels + numPixels/4;
		m_pitch[0] = res.width;
		m_pitch[1] = res.width/2;
		m_pitch[2] = res.width/2;
	}

	Resolution getResolution() const {
		return m_res;
	}

private:
	Resolution m_res;
	size_t m_pitch[3];
	uint8_t* m_comp[3];
};

static const Resolution VIDEO_RESOLUTION(320, 200);
static const int VIDEO_FPS = 24;

}
