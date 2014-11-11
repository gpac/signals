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
	  m_Swr(new ffpp::SwResampler) {
	AVSampleFormat avSrcFmt, avDstFmt;
	uint64_t avSrcChannelLayout, avDstChannelLayout;
	int avSrcNumChannels, avDstNumChannels, avSrcSampleRate, avDstSampleRate;

	libavAudioCtxConvertLibav(srcPcmFormat.get(), avSrcSampleRate, (int&)avSrcFmt, avSrcNumChannels, avSrcChannelLayout);
	libavAudioCtxConvertLibav(dstPcmFormat.get(), avDstSampleRate, (int&)avDstFmt, avDstNumChannels, avDstChannelLayout);
	
	m_Swr->setInputSampleFmt(avSrcFmt);
	m_Swr->setInputLayout(avSrcChannelLayout);
	m_Swr->setInputSampleRate(avSrcSampleRate);
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

	auto const srcNumSamples = audioData->size() / audioData->getFormat().getBytesPerSample();
	assert((srcNumSamples * dstPcmFormat->sampleRate) % srcPcmFormat->sampleRate == 0);
	auto const dstNumSamples = divUp(srcNumSamples * dstPcmFormat->sampleRate, (uint64_t)srcPcmFormat->sampleRate);

	auto const dstBufferSize = dstNumSamples * dstPcmFormat->getBytesPerSample();
	auto out = safe_cast<PcmData>(pins[0]->getBuffer(dstBufferSize));
	auto pDst = (uint8_t**)out->getPlanes();

	/*auto const numSamples = */m_Swr->convert(pDst, (int)dstNumSamples, (const uint8_t**)audioData->getPlanes(), (int)srcNumSamples);
	assert(m_Swr->getDelay(IClock::Rate) == 0); //with delay, flush at the end by setting srcNumSamples and dstNumSamples to 0
	//auto const sampleSize = numSamples * dstPcmFormat->getBytesPerSample();
	//assert(data->getTime() == sampleSize * IClock::Rate / dstPcmFormat->sampleRate);
	
	out->setTime(data->getTime());
	pins[0]->emit(out);
}

}
}
