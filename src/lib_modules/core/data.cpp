#include "data.hpp"

namespace Modules {

	Picture* Picture::create(const Resolution &res, const PixelFormat &format) {
		switch (format) {
		case YUV420:
			return new PictureYUV420(res);
			//TODO case RGB24:
			//	return new PictureRGB24(res);
		default:
			throw std::runtime_error("Unknown pixel format for Picture. Please contact your vendor");
		}
	}

}
