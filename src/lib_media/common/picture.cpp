#include "picture.hpp"

namespace Modules {
std::shared_ptr<Picture> Picture::create(PinPicture *pin, const Resolution &res, const PixelFormat &format) {
	std::shared_ptr<Picture> r;
	auto const size = PictureFormat::getSize(res, format);
	switch (format) {
	case YUV420P: r = safe_cast<Picture>(pin->getBuffer<PictureYUV420P>(size)); break;
	case YUYV422: r = safe_cast<Picture>(pin->getBuffer<PictureYUYV422>(size)); break;
	case RGB24: r = safe_cast<Picture>(pin->getBuffer<PictureRGB24>(size)); break;
	default: throw std::runtime_error("Unknown pixel format for Picture. Please contact your vendor");
	}
	r->setResolution(res);
	return r;
}
}
