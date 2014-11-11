#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "audio_convert.hpp"
#include "ffpp.hpp"
#include "../common/libav.hpp"
#include "../common/pcm.hpp"

namespace Modules {
namespace Transform {

AudioConvert::AudioConvert(AudioSampleFormat srcFmt, AudioLayout srcChannelLayout, uint32_t srcSampleRate, AudioStruct srcStruct,
	AudioSampleFormat dstFmt, AudioLayout dstChannelLayout, uint32_t dstSampleRate, AudioStruct dstStruct)
	: srcPcmFormat(new PcmFormat(srcSampleRate, srcChannelLayout, srcFmt, srcStruct)),
	  dstPcmFormat(new PcmFormat(dstSampleRate, dstChannelLayout, dstFmt, dstStruct)),
	  aFrame(new ffpp::Frame), m_Swr(new ffpp::SwResampler) {
	AVSampleFormat avDstFmt;
	uint64_t avDstChannelLayout;
	int avDstNumChannels, avDstSampleRate;

	libavAudioCtxConvertLibav(srcPcmFormat.get(), aFrame->get()->sample_rate, aFrame->get()->format, aFrame->get()->channels, aFrame->get()->channel_layout);
	libavAudioCtxConvertLibav(dstPcmFormat.get(), avDstSampleRate, (int&)avDstFmt, avDstNumChannels, avDstChannelLayout);
	
	m_Swr->setInputSampleFmt((AVSampleFormat)aFrame->get()->format);
	m_Swr->setInputLayout(aFrame->get()->channel_layout);
	m_Swr->setInputSampleRate(aFrame->get()->sample_rate);
	m_Swr->setOutputSampleFmt(avDstFmt);
	m_Swr->setOutputLayout(avDstChannelLayout);
	m_Swr->setOutputSampleRate(avDstSampleRate);
	m_Swr->init();

	PinPcmFactory factory;
	pins.push_back(uptr(factory.createPin()));
}

AudioConvert::~AudioConvert() {
}

void AudioConvert::process(std::shared_ptr<Data> data) {
	auto audioData = safe_cast<PcmData>(data);
	if (audioData->getFormat() != *srcPcmFormat)
		throw std::runtime_error("[AudioConvert] Incompatible input audio data");
	libavFrameDataConvert(audioData.get(), aFrame->get());

	auto const srcNumSamples = aFrame->get()->nb_samples;
	auto const dstNumSamples = divUp(srcNumSamples * dstPcmFormat->sampleRate, srcPcmFormat->sampleRate);

	const int bufferSize = av_samples_get_buffer_size(nullptr, aFrame->get()->channels, aFrame->get()->nb_samples, (AVSampleFormat)aFrame->get()->format, 0);
	auto out(pins[0]->getBuffer(bufferSize * 10)); //Romain: pourquoi *10?
	uint8_t* pDst = out->data();

	/*auto const numSamples = */m_Swr->convert(&pDst, dstNumSamples, (const uint8_t**)aFrame->get()->data, srcNumSamples);
	assert(m_Swr->getDelay(IClock::Rate) == 0);
	//auto const sampleSize = numSamples * dstPcmFormat->getBytesPerSample();
	//assert(data->getTime() == sampleSize * IClock::Rate / dstPcmFormat->sampleRate);
	
	out->setTime(data->getTime());
	pins[0]->emit(out);
}

}
}
