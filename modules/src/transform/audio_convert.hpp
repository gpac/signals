#pragma once

#include "internal/core/module.hpp"
extern "C" {
#include <libavutil/samplefmt.h> //TODO: remove when media types
}
#include <string>


namespace ffpp {
struct SwResampler;
}

namespace Modules {
namespace Transform {

class AudioConvert : public Module {
	public:
		AudioConvert(enum AVSampleFormat srcFmt, uint64_t srcChannelLayout, int srcSampleRate, enum AVSampleFormat dstFmt, uint64_t dstChannelLayout, int dstSampleRate);
		~AudioConvert();
		void process(std::shared_ptr<Data> data);

	private:
		AVSampleFormat srcFmt;
		uint64_t srcChannelLayout;
		int srcSampleRate;
		AVSampleFormat dstFmt;
		uint64_t dstChannelLayout;
		int dstSampleRate;
		std::unique_ptr<ffpp::SwResampler> const m_Swr;
};

}
}
