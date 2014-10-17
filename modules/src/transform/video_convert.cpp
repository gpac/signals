#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "video_convert.hpp"
#include "../common/libav.hpp"

namespace Modules {
namespace Transform {

VideoConvert::VideoConvert(int srcW, int srcH, AVPixelFormat srcFormat, int dstW, int dstH, AVPixelFormat dstFormat)
	: srcW(srcW), srcH(srcH), dstW(dstW), dstH(dstH), dstFormat(dstFormat) {
	m_SwContext = sws_getContext(srcW, srcH, srcFormat, dstW, dstH, dstFormat, SWS_BILINEAR, nullptr, nullptr, nullptr);
	signals.push_back(uptr(pinFactory->createPin()));
}

VideoConvert::~VideoConvert() {
	sws_freeContext(m_SwContext);
}

void VideoConvert::process(std::shared_ptr<Data> data) {
	uint8_t *srcSlice[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	int srcStride[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	/*TODO: autoconversion to DataAVFrame*/
	{
		auto videoData = dynamic_cast<DataAVFrame*>(data.get());
		if (videoData) {
			memcpy(srcSlice , videoData->getFrame()->data    , sizeof(srcSlice));
			memcpy(srcStride, videoData->getFrame()->linesize, sizeof(srcStride));
		} else {
			if (srcW * srcH * 3 != data->size()) {
				Log::msg(Log::Warning, "[VideoConvert] Invalid data type.");
				return;
			}
			srcSlice[0]  = data->data();
			srcStride[0] = srcW * 3;
		}
	}

	const int dstFrameSize = avpicture_get_size(dstFormat, dstW, dstH);
	auto out(signals[0]->getBuffer(dstFrameSize));

	uint8_t* pDst[3] = { nullptr, nullptr, nullptr };
	int dstStride[3] = { 0, 0, 0 };
	switch (dstFormat) {
	case AV_PIX_FMT_YUV420P:
		pDst[0] = out->data();
		pDst[1] = out->data() + dstW * dstH;
		pDst[2] = out->data() + dstW * dstH * 5 / 4;
		dstStride[0] = dstW;
		dstStride[1] = dstW / 2;
		dstStride[2] = dstW / 2;
		break;
	case AV_PIX_FMT_RGB24:
		pDst[0] = out->data();
		dstStride[0] = dstW * 3;
		break;
	default:
		assert(0);
		Log::msg(Log::Warning, "[VideoConvert] Dst colorspace not supported. Ignoring.");
		return;
	}

	sws_scale(m_SwContext, srcSlice, srcStride, 0, srcH, pDst, dstStride);

	signals[0]->emit(out);
}

}
}
