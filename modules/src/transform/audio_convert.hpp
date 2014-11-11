#pragma once

#include "internal/core/module.hpp"
#include "../common/pcm.hpp"
extern "C" {
#include <libavutil/samplefmt.h> //TODO: remove when media types
}


namespace ffpp {
struct SwResampler;
struct Frame;
}

namespace Modules {
namespace Transform {

class AudioConvert : public Module {
	public:
		AudioConvert(AudioSampleFormat srcFmt, AudioLayout srcChannelLayout, uint32_t srcSampleRate, AudioStruct srcStruct,
			         AudioSampleFormat dstFmt, AudioLayout dstChannelLayout, uint32_t dstSampleRate, AudioStruct dstStruct);
		~AudioConvert();
		void process(std::shared_ptr<Data> data) override;

	private:
		std::unique_ptr<PcmFormat> srcPcmFormat, dstPcmFormat;
		std::unique_ptr<ffpp::SwResampler> const m_Swr;
};

}
}
