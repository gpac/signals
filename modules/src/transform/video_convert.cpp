#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "video_convert.hpp"
#include "../common/libav.hpp"

namespace Modules {
namespace Transform {

VideoConvert::VideoConvert(Resolution srcRes, AVPixelFormat srcFormat, Resolution dstRes, AVPixelFormat dstFormat)
	: srcRes(srcRes), dstRes(dstRes), dstFormat(dstFormat) {
	m_SwContext = sws_getContext(srcRes.width, srcRes.height, srcFormat, dstRes.width, dstRes.height, dstFormat, SWS_BILINEAR, nullptr, nullptr, nullptr);
	PinDefaultFactory pinFactory;
	pins.push_back(uptr(pinFactory.createPin()));
}

VideoConvert::~VideoConvert() {
	sws_freeContext(m_SwContext);
}

void VideoConvert::process(std::shared_ptr<Data> data) {
	uint8_t *srcSlice[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	int srcStride[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	auto videoData = safe_cast<Picture>(data);
	for(int i=0;i < 3; ++i)
	{
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
			pDst[0] = pic->data();
			pDst[1] = pic->data() + dstRes.width * dstRes.height;
			pDst[2] = pic->data() + dstRes.width * dstRes.height * 5 / 4;
			dstStride[0] = dstRes.width;
			dstStride[1] = dstRes.width / 2;
			dstStride[2] = dstRes.width / 2;
			out = pic;
			break;
		}
	case AV_PIX_FMT_RGB24: {
			const int dstFrameSize = avpicture_get_size(dstFormat, dstRes.width, dstRes.height);
			out = rawAlloc.getBuffer(dstFrameSize);
			pDst[0] = out->data();
			dstStride[0] = dstRes.width * 3;
			break;
		}
	default:
		assert(0);
		Log::msg(Log::Warning, "[VideoConvert] Dst colorspace not supported. Ignoring.");
		return;
	}

	sws_scale(m_SwContext, srcSlice, srcStride, 0, srcRes.height, pDst, dstStride);

	pins[0]->emit(out);
}

}
}
