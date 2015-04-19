#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "video_convert.hpp"
#include "lib_ffpp/ffpp.hpp"
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
	auto input = addInput(new Input<DataPicture>(this));
	input->setMetadata(new MetadataRawVideo);
	output = addOutput(new OutputPicture);
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

void VideoConvert::process(Data data) {
	auto videoData = safe_cast<const DataPicture>(data);
	if (videoData->getFormat() != srcFormat) {
		if (m_SwContext)
			Log::msg(Log::Info, "[VideoConvert] Incompatible input video data. Reconfiguring.");
		reconfigure(videoData->getFormat());
	}

	uint8_t const* srcSlice[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	int srcStride[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for (int i=0; i<3; ++i) {
		srcSlice[i] = videoData->getPlane(i);
		srcStride[i] = (int)videoData->getPitch(i);
	}

	std::shared_ptr<DataBase> out;
	uint8_t* pDst[3] = { nullptr, nullptr, nullptr };
	int dstStride[3] = { 0, 0, 0 };
	switch (dstFormat.format) {
	case YUV420P:
	case YUYV422:
	case RGB24: {
			auto pic = DataPicture::create(output, dstFormat.res, dstFormat.format);
			for (size_t i=0; i<pic->getNumPlanes(); ++i) {
				pDst[i] = pic->getPlane(i);
				dstStride[i] = (int)pic->getPitch(i);
			}
			out = pic;
			break;
		}
	default:
		throw std::runtime_error("[VideoConvert] Destination colorspace not supported.");
	}

	sws_scale(m_SwContext, srcSlice, srcStride, 0, srcFormat.res.height, pDst, dstStride);

	output->emit(out);
}

}
}
