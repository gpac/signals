#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "video_convert.hpp"
#include "../common/libav.hpp"

namespace Modules {
namespace Transform {

VideoConvert::VideoConvert(Resolution dstRes, AVPixelFormat dstFormat)
: m_SwContext(nullptr), dstRes(dstRes), dstFormat(dstFormat), picAlloc(ALLOC_NUM_BLOCKS_DEFAULT), rawAlloc(ALLOC_NUM_BLOCKS_DEFAULT) {
	memset(&m_srcRes, 0, sizeof(m_srcRes));
	output = addPin(new PinDefault);
}

void VideoConvert::reconfigure(const Resolution &srcRes, const AVPixelFormat &srcFormat) {
	if (m_SwContext)
		sws_freeContext(m_SwContext);
	m_SwContext = sws_getContext(srcRes.width, srcRes.height, srcFormat, dstRes.width, dstRes.height, dstFormat, SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!m_SwContext)
		throw std::runtime_error("[VideoConvert] Impossible to set up video converter.");
	m_srcRes = srcRes;
}

VideoConvert::~VideoConvert() {
	sws_freeContext(m_SwContext);
}

void VideoConvert::process(std::shared_ptr<const Data> data) {
	auto videoData = safe_cast<const Picture>(data);
	if (videoData->getResolution() != m_srcRes) {
		if (m_SwContext)
			Log::msg(Log::Info, "[VideoConvert] Incompatible input video data. Reconfiguring.");
		reconfigure(videoData->getResolution(), AV_PIX_FMT_YUV420P);
	}

	uint8_t *srcSlice[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	int srcStride[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for (int i=0; i<3; ++i) {
		srcSlice[i] = videoData->getComp(i);
		srcStride[i] = (int)videoData->getPitch(i);
	}

	std::shared_ptr<Data> out;
	uint8_t* pDst[3] = { nullptr, nullptr, nullptr };
	int dstStride[3] = { 0, 0, 0 };
	switch (dstFormat) {
	case AV_PIX_FMT_YUV420P: {
			auto pic = picAlloc.getBuffer(0);
			pic->setResolution(dstRes);
			for (int i=0; i<3; ++i) {
				pDst[i] = pic->getComp(i);
				dstStride[i] = (int)pic->getPitch(i);
			}
			out = pic;
			break;
		}
	case AV_PIX_FMT_RGB24: {
			const int dstFrameSize = avpicture_get_size(dstFormat, dstRes.width, dstRes.height);
			auto raw = rawAlloc.getBuffer(dstFrameSize);
			pDst[0] = raw->data();
			dstStride[0] = dstRes.width * 3;
			out = raw;
			break;
		}
	default:
		assert(0);
		Log::msg(Log::Warning, "[VideoConvert] Dst colorspace not supported. Ignoring.");
		return;
	}

	sws_scale(m_SwContext, srcSlice, srcStride, 0, m_srcRes.height, pDst, dstStride);

	output->emit(out);
}

}
}
