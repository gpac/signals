#pragma once

#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

namespace ffpp {
	struct Frame {
		Frame() {
			avFrame = avcodec_alloc_frame();
			if(!avFrame)
				throw std::runtime_error("Frame allocation failed");
		}

		~Frame() {
			avcodec_free_frame(&avFrame);
		}

		AVFrame* operator->() {
			return avFrame;
		}

		AVFrame* get() {
			return avFrame;
		}

	private:
		AVFrame* avFrame;
	};
}

