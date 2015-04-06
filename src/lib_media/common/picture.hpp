#pragma once

#include "lib_modules/core/data.hpp"

namespace Modules {

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
	UNKNOWN = 0,
	YUV420,
	RGB24
};

class PictureFormat {
public:
	PictureFormat() : format(UNKNOWN) {
	}
	PictureFormat(const Resolution &res, const PixelFormat &format)
	: res(res), format(format) {
	}
	bool operator==(PictureFormat const& other) const {
		return res == other.res && format == other.format;
	}
	bool operator!=(PictureFormat const& other) const {
		return !(*this == other);
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
	PictureFormat getFormat() const {
		return m_format;
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
