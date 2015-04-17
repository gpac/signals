#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "audio_convert.hpp"
#include "lib_ffpp/ffpp.hpp"
#include "../common/libav.hpp"
#include "../common/pcm.hpp"

namespace Modules {
namespace Transform {

AudioConvert::AudioConvert(const PcmFormat &dstFormat)
: dstPcmFormat(dstFormat), m_Swr(nullptr), accumulatedTimeInDstSR(0), autoConfigure(true) {
	memset(&srcPcmFormat, 0, sizeof(srcPcmFormat));
	output = addOutput(new OutputPcm);
}

AudioConvert::AudioConvert(const PcmFormat &srcFormat, const PcmFormat &dstFormat)
: srcPcmFormat(srcFormat), dstPcmFormat(dstFormat),
  m_Swr(new ffpp::SwResampler), accumulatedTimeInDstSR(0), autoConfigure(false) {
	configure(srcPcmFormat);
	output = addOutput(new OutputPcm);
}

AudioConvert::~AudioConvert() {
}

void AudioConvert::reconfigure(const PcmFormat &srcFormat) {
	flush();
	m_Swr = uptr(new ffpp::SwResampler);
	configure(srcFormat);
	srcPcmFormat = srcFormat;
}

void AudioConvert::configure(const PcmFormat &srcFormat) {
	AVSampleFormat avSrcFmt, avDstFmt;
	uint64_t avSrcChannelLayout, avDstChannelLayout;
	int avSrcNumChannels, avDstNumChannels, avSrcSampleRate, avDstSampleRate;
	libavAudioCtxConvertLibav(&srcFormat   , avSrcSampleRate, (int&)avSrcFmt, avSrcNumChannels, avSrcChannelLayout);
	libavAudioCtxConvertLibav(&dstPcmFormat, avDstSampleRate, (int&)avDstFmt, avDstNumChannels, avDstChannelLayout);

	m_Swr->setInputSampleFmt(avSrcFmt);
	m_Swr->setInputLayout(avSrcChannelLayout);
	m_Swr->setInputSampleRate(avSrcSampleRate);
	m_Swr->setOutputSampleFmt(avDstFmt);
	m_Swr->setOutputLayout(avDstChannelLayout);
	m_Swr->setOutputSampleRate(avDstSampleRate);
	m_Swr->init();
}

void AudioConvert::flush() {
	if (m_Swr.get())
		process(nullptr);
}

void AudioConvert::process(std::shared_ptr<const Data> data) {
	uint64_t srcNumSamples, dstNumSamples;
	uint8_t * const * pSrc;
	auto audioData = safe_cast<const PcmData>(data);
	if (audioData) {
		if (audioData->getFormat() != srcPcmFormat) {
			if (autoConfigure) {
				Log::msg(Log::Info, "[AudioConvert] Incompatible input audio data. Reconfiguring.");
				reconfigure(audioData->getFormat());
			} else {
				throw std::runtime_error("[AudioConvert] Incompatible input audio data.");
			}
			accumulatedTimeInDstSR = clockToTimescale(data->getTime(), srcPcmFormat.sampleRate);
		}

		srcNumSamples = audioData->size() / audioData->getFormat().getBytesPerSample();
		dstNumSamples = divUp(srcNumSamples * dstPcmFormat.sampleRate, (uint64_t)srcPcmFormat.sampleRate);
		pSrc = audioData->getPlanes();
	} else {
		dstNumSamples = m_Swr->getDelay(dstPcmFormat.sampleRate);
		if (dstNumSamples == 0)
			return;
		pSrc = nullptr;
		srcNumSamples = 0;
	}

	auto const dstBufferSize = dstNumSamples * dstPcmFormat.getBytesPerSample();
	auto out = output->getBuffer(0);
	out->setFormat(dstPcmFormat);
	for (uint8_t i=0; i < dstPcmFormat.numPlanes; ++i)
		out->setPlane(i, nullptr, dstBufferSize / dstPcmFormat.numPlanes);

	auto pDst = (uint8_t**)out->getPlanes();
	auto const outNumSamples = m_Swr->convert(pDst, (int)dstNumSamples, (const uint8_t**)pSrc, (int)srcNumSamples);

	auto const outPlaneSize = outNumSamples * dstPcmFormat.getBytesPerSample() / dstPcmFormat.numPlanes;
	for (uint8_t i = 0; i < dstPcmFormat.numPlanes; ++i)
		out->setPlane(i, out->getPlane(i), outPlaneSize);

	accumulatedTimeInDstSR += outNumSamples;
	auto const accumulatedTimeIn180k = timescaleToClock(accumulatedTimeInDstSR, dstPcmFormat.sampleRate);
	out->setTime(accumulatedTimeIn180k);

	output->emit(out);
}

}
}
