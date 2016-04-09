#pragma once

#include "lib_modules/core/data.hpp"
#include "lib_modules/core/output.hpp"

namespace Modules {

struct Resolution {
	Resolution() : width(0), height(0) {
	}
	Resolution(unsigned int w, unsigned int h) : width(w), height(h) {
	}
	bool operator==(Resolution const& other) const {
		return width == other.width && height == other.height;
	}
	bool operator!=(Resolution const& other) const {
		return !(*this == other);
	}
	Resolution operator/(const int div) const {
		return Resolution(this->width/2, this->height/2);
	}
	unsigned int width, height;
	std::string toString() const {
		std::stringstream ss;
		ss << width << "x" << height;
		return ss.str();
	}
};

#undef PixelFormat //there are collisions with FFmpeg here
enum PixelFormat {
	UNKNOWN_PF = -1,
	YUV420P,
	YUYV422,
	RGB24
};

class PictureFormat {
	public:
		PictureFormat() : format(UNKNOWN_PF) {
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
			case YUV420P: return res.width * res.height * 3 / 2;
			case YUYV422: return res.width * res.height * 2;
			case RGB24: return res.width * res.height * 3;
			default:
				throw std::runtime_error("Unknown pixel format. Please contact your vendor.");
			}
		}

		Resolution res;
		PixelFormat format;
};

class DataPicture;
typedef OutputDataDefault<DataPicture> OutputPicture;

//TODO: we should probably separate planar vs non-planar data, avoid resize on the data, etc.
class DataPicture : public DataRaw {
	public:
		DataPicture(size_t unused) : DataRaw(0) {}
		static std::shared_ptr<DataPicture> create(OutputPicture *out, const Resolution &res, const PixelFormat &format);

		virtual bool isRecyclable() const override {
			return true;
		}

		PictureFormat getFormat() const {
			return m_format;
		}
		size_t getSize() const {
			return m_format.getSize();
		}
		virtual size_t getNumPlanes() const = 0;
		virtual const uint8_t* getPlane(size_t planeIdx) const = 0;
		virtual uint8_t* getPlane(size_t planeIdx) = 0;
		virtual size_t getPitch(size_t planeIdx) const = 0;
		virtual void setResolution(const Resolution &res) = 0;

	protected:
		DataPicture(const Resolution &res, const PixelFormat &format)
			: DataRaw(PictureFormat::getSize(res, format)),
			  m_format(res, format) {
		}

		PictureFormat m_format;
};

class PictureYUV420P : public DataPicture {
	public:
		PictureYUV420P(size_t unused) : DataPicture(0) {
			m_format.format = YUV420P;
		}
		PictureYUV420P(const Resolution &res)
			: DataPicture(res, YUV420P) {
			setResolution(res);
		}
		size_t getNumPlanes() const override {
			return 3;
		}
		const uint8_t* getPlane(size_t planeIdx) const override {
			return m_planes[planeIdx];
		}
		uint8_t* getPlane(size_t planeIdx) override {
			return m_planes[planeIdx];
		}
		size_t getPitch(size_t planeIdx) const override {
			return m_pitch[planeIdx];
		}
		void setResolution(const Resolution &res) override {
			m_format.res = res;
			resize(m_format.getSize());
			auto const numPixels = res.width * res.height;
			m_planes[0] = data();
			m_planes[1] = data() + numPixels;
			m_planes[2] = data() + numPixels + numPixels / 4;
			m_pitch[0] = res.width;
			m_pitch[1] = res.width / 2;
			m_pitch[2] = res.width / 2;
		}

	private:
		size_t m_pitch[3];
		uint8_t* m_planes[3];
};

class PictureYUYV422 : public DataPicture {
	public:
		PictureYUYV422(size_t unused) : DataPicture(0) {
			m_format.format = YUYV422;
		}
		PictureYUYV422(const Resolution &res)
			: DataPicture(res, YUYV422) {
			setResolution(res);
		}
		size_t getNumPlanes() const override {
			return 1;
		}
		const uint8_t* getPlane(size_t planeIdx) const override {
			return data();
		}
		uint8_t* getPlane(size_t planeIdx) override {
			return data();
		}
		size_t getPitch(size_t planeIdx) const override {
			return m_format.res.width * 2;
		}
		void setResolution(const Resolution &res) override {
			m_format.res = res;
			resize(m_format.getSize());
		}
};

class PictureRGB24 : public DataPicture {
	public:
		PictureRGB24(size_t unused) : DataPicture(0) {
			m_format.format = RGB24;
		}
		PictureRGB24(const Resolution &res)
			: DataPicture(res, RGB24) {
			setResolution(res);
		}
		size_t getNumPlanes() const override {
			return 1;
		}
		const uint8_t* getPlane(size_t planeIdx) const override {
			return data();
		}
		uint8_t* getPlane(size_t planeIdx) override {
			return data();
		}
		size_t getPitch(size_t planeIdx) const override {
			return m_format.res.width * 3;
		}
		void setResolution(const Resolution &res) override {
			m_format.res = res;
			resize(m_format.getSize());
		}
};

static const Resolution VIDEO_RESOLUTION(320, 180);
static const int VIDEO_FPS = 24;

}
