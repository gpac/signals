#pragma once

#include "clock.hpp"

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>

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
	uint8_t const* data() const {
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
	std::string toString() const {
		std::stringstream ss;
		ss << width << "x" << height;
		return ss.str();
	}
};

enum PixelFormat {
	UNKNOWN2 = 0,
	YUV420,
	RGB24
};

class PictureFormat {
public:
	PictureFormat() = default;
	PictureFormat(const Resolution &res, const PixelFormat &format)
	: res(res), format(format) {
	}

	size_t getSize() const {
		return getSize(res, format);
	}

	static size_t getSize(const Resolution &res, const PixelFormat &format) {
		switch (format) {
		case YUV420:
			return res.width * res.height * 3 / 2;
		default:
			throw std::runtime_error("Unknown pixel format. Please contact your vendor.");
		}
	}

	Resolution res;
	PixelFormat format;
};

class Picture : public RawData {
public:
	Picture(size_t unused) : RawData(0) {}
	static Picture* create(const Resolution &res, const PixelFormat &format);
	Resolution getResolution() const {
		return m_format.res;
	}
	PixelFormat getFormat() const {
		return m_format.format;
	}
	size_t getSize() const {
		return m_format.getSize();
	}

	virtual uint8_t* getComp(int comp) const = 0;
	virtual size_t getPitch(int comp) const = 0;
	virtual void setResolution(const Resolution &res) = 0;

protected:
	Picture(const Resolution &res, const PixelFormat &format)
		: RawData(PictureFormat::getSize(res, format)),
		m_format(res, format) {
	}

	PictureFormat m_format;
};

class PictureYUV420 : public Picture {
public:
	PictureYUV420(size_t unused) : Picture(0) {}
	PictureYUV420(const Resolution &res)
	: Picture(res, YUV420) {
		setResolution(res);
	}
	uint8_t* getComp(int comp) const override {
		return m_comp[comp];
	}
	size_t getPitch(int comp) const override {
		return m_pitch[comp];
	}
	void setResolution(const Resolution &res) override {
		m_format.res = res;
		resize(m_format.getSize());
		auto const numPixels = res.width * res.height;
		m_comp[0] = data();
		m_comp[1] = data() + numPixels;
		m_comp[2] = data() + numPixels + numPixels / 4;
		m_pitch[0] = res.width;
		m_pitch[1] = res.width / 2;
		m_pitch[2] = res.width / 2;
	}

private:
	size_t m_pitch[3];
	uint8_t* m_comp[3];
};

static const Resolution VIDEO_RESOLUTION(320, 180);
static const int VIDEO_FPS = 24;

}
