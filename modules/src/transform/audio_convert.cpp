#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "audio_convert.hpp"
#include "ffpp.hpp"
#include "../common/libav.hpp"
#include "../common/pcm.hpp"

namespace Modules {
namespace Transform {

AudioConvert::AudioConvert(PcmFormat srcFormat, PcmFormat dstFormat) 
	: srcPcmFormat(srcFormat),
	  dstPcmFormat(dstFormat),
	  m_Swr(new ffpp::SwResampler), accumulatedTimeInDstSR(0) {
	AVSampleFormat avSrcFmt, avDstFmt;
	uint64_t avSrcChannelLayout, avDstChannelLayout;
	int avSrcNumChannels, avDstNumChannels, avSrcSampleRate, avDstSampleRate;

	libavAudioCtxConvertLibav(&srcPcmFormat, avSrcSampleRate, (int&)avSrcFmt, avSrcNumChannels, avSrcChannelLayout);
	libavAudioCtxConvertLibav(&dstPcmFormat, avDstSampleRate, (int&)avDstFmt, avDstNumChannels, avDstChannelLayout);
	
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
	//TODO: process(nullptr);
}

void AudioConvert::process(std::shared_ptr<Data> data) {
	uint64_t srcNumSamples, dstNumSamples;
	auto audioData = safe_cast<PcmData>(data);
	if (audioData) {
		if (audioData->getFormat() != srcPcmFormat)
			throw std::runtime_error("[AudioConvert] Incompatible input audio data");

		srcNumSamples = audioData->size() / audioData->getFormat().getBytesPerSample();
		dstNumSamples = divUp(srcNumSamples * dstPcmFormat.sampleRate, (uint64_t)srcPcmFormat.sampleRate);
	} else {
		srcNumSamples = 0;
		dstNumSamples = m_Swr->getDelay(dstPcmFormat.sampleRate);
		if (dstNumSamples == 0)
			return;
	}

	auto const dstBufferSize = dstNumSamples * dstPcmFormat.getBytesPerSample();
	auto out = safe_cast<PcmData>(pins[0]->getBuffer(0));
	out->setFormat(dstPcmFormat);
	for (uint8_t i=0; i < dstPcmFormat.numPlanes; ++i)
		out->setPlane(i, nullptr, dstBufferSize / dstPcmFormat.numPlanes);
	auto pDst = (uint8_t**)out->getPlanes();

	auto const outNumSamples = m_Swr->convert(pDst, (int)dstNumSamples, (const uint8_t**)audioData->getPlanes(), (int)srcNumSamples);

	auto const outPlaneSize = outNumSamples * dstPcmFormat.getBytesPerSample() / dstPcmFormat.numPlanes;
	for (uint8_t i = 0; i < dstPcmFormat.numPlanes; ++i)
		out->setPlane(i, out->getPlane(i), outPlaneSize);

	accumulatedTimeInDstSR += outNumSamples;
	auto const accumulatedTimeIn180k = divUp<uint64_t>(accumulatedTimeInDstSR * IClock::Rate, dstPcmFormat.sampleRate);
	out->setTime(accumulatedTimeIn180k);

	pins[0]->emit(out);
}

}
}
