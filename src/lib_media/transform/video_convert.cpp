#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "video_convert.hpp"
#include "../common/libav.hpp"

namespace {
AVPixelFormat libavPixFmtConvert(const Modules::PixelFormat format) {
	AVPixelFormat pixFmt;
	pixelFormat2libavPixFmt(format, pixFmt);
	return pixFmt;
}
}

namespace Modules {
namespace Transform {

VideoConvert::VideoConvert(const PictureFormat &dstFormat)
: m_SwContext(nullptr), dstFormat(dstFormat) {
	output = addPin(new PinDefault);
}

void VideoConvert::reconfigure(const PictureFormat &format) {
	sws_freeContext(m_SwContext);
	m_SwContext = sws_getContext(format.res.width, format.res.height, libavPixFmtConvert(format.format),
		                         dstFormat.res.width, dstFormat.res.height, libavPixFmtConvert(dstFormat.format),
								 SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!m_SwContext)
		throw std::runtime_error("[VideoConvert] Impossible to set up video converter.");
	srcFormat = format;
}

VideoConvert::~VideoConvert() {
	sws_freeContext(m_SwContext);
}

void VideoConvert::process(std::shared_ptr<const Data> data) {
	auto videoData = safe_cast<const Picture>(data);
	if (videoData->getFormat() != srcFormat) {
		if (m_SwContext)
			Log::msg(Log::Info, "[VideoConvert] Incompatible input video data. Reconfiguring.");
		reconfigure(videoData->getFormat());
	}

	uint8_t const* srcSlice[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	int srcStride[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for (int i=0; i<3; ++i) {
		srcSlice[i] = videoData->getComp(i);
		srcStride[i] = (int)videoData->getPitch(i);
	}

	std::shared_ptr<Data> out;
	uint8_t* pDst[3] = { nullptr, nullptr, nullptr };
	int dstStride[3] = { 0, 0, 0 };
	switch (dstFormat.format) {
	case YUV420: {
			auto pic = picAlloc.getBuffer(0);
			pic->setResolution(dstFormat.res);
			for (int i=0; i<3; ++i) {
				pDst[i] = pic->getComp(i);
				dstStride[i] = (int)pic->getPitch(i);
			}
			out = pic;
			break;
		}
	case RGB24: {
			auto pic = rawAlloc.getBuffer(0);
			pic->setResolution(dstFormat.res);
			pDst[0] = pic->getComp(0);
			dstStride[0] = (int)pic->getPitch(0);
			out = pic;
			break;
		}
	default:
		assert(0);
		Log::msg(Log::Warning, "[VideoConvert] Dst colorspace not supported. Ignoring.");
		return;
	}

	sws_scale(m_SwContext, srcSlice, srcStride, 0, srcFormat.res.height, pDst, dstStride);

	output->emit(out);
}

}
}
