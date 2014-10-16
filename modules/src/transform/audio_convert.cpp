#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "audio_convert.hpp"
#include "ffpp.hpp"
#include "../common/libav.hpp"
#include "../common/pcm.hpp"

namespace Modules {
namespace Transform {

AudioConvert::AudioConvert(AVSampleFormat srcFmt, uint64_t srcChannelLayout, int srcSampleRate, AVSampleFormat dstFmt, uint64_t dstChannelLayout, int dstSampleRate)
	: Module(new PinPcmFactory), srcFmt(srcFmt), srcChannelLayout(srcChannelLayout), srcSampleRate(srcSampleRate),
	  dstFmt(dstFmt), dstChannelLayout(dstChannelLayout), dstSampleRate(dstSampleRate), m_Swr(new ffpp::SwResampler) {
	m_Swr->setInputSampleFmt(srcFmt);
	m_Swr->setInputLayout(srcChannelLayout);
	m_Swr->setInputSampleRate(srcSampleRate);
	m_Swr->setOutputSampleFmt(dstFmt);
	m_Swr->setOutputLayout(dstChannelLayout);
	m_Swr->setOutputSampleRate(dstSampleRate);
	m_Swr->init();
	signals.push_back(uptr(pinFactory->createPin()));
}

AudioConvert::~AudioConvert() {
}

void AudioConvert::process(std::shared_ptr<Data> data) {
	auto audioData = dynamic_cast<DataAVFrame*>(data.get());
	if (!audioData) {
		Log::msg(Log::Warning, "[AudioConvert] Invalid data type.");
		return;
	}

	const int bufferSize = av_samples_get_buffer_size(nullptr, av_get_channel_layout_nb_channels(srcChannelLayout), audioData->getFrame()->nb_samples, srcFmt, 0);

	auto const srcNumSamples = audioData->getFrame()->nb_samples;
	auto const dstNumSamples = divUp(srcNumSamples * dstSampleRate, srcSampleRate);

	auto out(signals[0]->getBuffer(bufferSize * 10));

	uint8_t* pDst = out->data();
	auto const numSamples = m_Swr->convert(&pDst, dstNumSamples, (const uint8_t**)audioData->getFrame()->data, srcNumSamples);

	auto const dstChannels = av_get_channel_layout_nb_channels(dstChannelLayout);
	auto const sampleSize = av_samples_get_buffer_size(nullptr, dstChannels, numSamples, dstFmt, 1);

	out->resize(sampleSize);

	signals[0]->emit(out);
}

}
}
