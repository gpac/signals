#include "sdl_audio.hpp"
#include "render_common.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "SDL2/SDL.h"
#include <cstring>
#include <fstream>
#include <algorithm>

namespace Modules {

namespace {
SDL_AudioSpec SDLAudioSpecConvert(const PcmFormat *cfg) {
	SDL_AudioSpec audioSpec;
	audioSpec.freq = cfg->sampleRate;
	audioSpec.channels = cfg->numChannels;
	switch (cfg->sampleFormat) {
	case S16: audioSpec.format = AUDIO_S16; break;
	case F32: audioSpec.format = AUDIO_F32; break;
	default: throw std::runtime_error("Unknown SDL audio format");
	}

	return audioSpec;
}
}

namespace Render {

SDLAudio* SDLAudio::create() {
	return new SDLAudio();
}

SDLAudio::SDLAudio() : pcmFormat(new PcmFormat(44100, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved)), m_FifoTime(0) {
	SDL_AudioSpec audioSpec = SDLAudioSpecConvert(pcmFormat.get());
	audioSpec.samples = 1024;  /* Good low-latency value for callback */
	audioSpec.callback = &SDLAudio::staticFillAudio;
	audioSpec.userdata = this;
	bytesPerSample = pcmFormat->getBytesPerSample();

	SDL_AudioSpec realSpec;

	if (SDL_OpenAudio(&audioSpec, &realSpec) < 0) {
		Log::msg(Log::Warning, "[SDLAudio render] Couldn't open audio: %s", SDL_GetError());
		throw std::runtime_error("Audio output creation failed");
	}

	m_Latency = realSpec.samples * IClock::Rate / realSpec.freq;
	Log::msg(Log::Info, "[SDLAudio render] %s Hz %s ms", realSpec.freq, m_Latency * 1000.0f / IClock::Rate);

	SDL_PauseAudio(0);
}

SDLAudio::~SDLAudio() {
	size_t remaining;
	{
		std::lock_guard<std::mutex> lg(m_Mutex);
		remaining = m_Fifo.bytesToRead();
	}
	if (remaining > 0) {
		do {
			SDL_Delay(10);
			std::lock_guard<std::mutex> lg(m_Mutex);
			remaining = m_Fifo.bytesToRead();
		} while (remaining != 0);
	}
	SDL_CloseAudio();
}

void SDLAudio::process(std::shared_ptr<Data> data) {
	auto pcmData = safe_cast<PcmData>(data);
	if (pcmData->getFormat() != *pcmFormat)
		throw std::runtime_error("[SDLAudio] Incompatible audio data");

	{
		std::lock_guard<std::mutex> lg(m_Mutex);
		if(m_Fifo.bytesToRead() == 0) {
			m_FifoTime = pcmData->getTime() + PREROLL_DELAY;
		}
		m_Fifo.write(pcmData->data(), (size_t)pcmData->size());
	}
}

void SDLAudio::fillAudio(uint8_t *stream, int len) {
	// timestamp of the first sample of the buffer
	auto const bufferTimeIn180k = g_DefaultClock->now() + m_Latency;

	std::lock_guard<std::mutex> lg(m_Mutex);

	int64_t numSamplesToProduce = len / bytesPerSample;

	auto const relativeTimePosition = int64_t(m_FifoTime) - int64_t(bufferTimeIn180k);
	auto const relativeSamplePosition = relativeTimePosition * pcmFormat->sampleRate / IClock::Rate;

	if (relativeSamplePosition < -audioJitterTolerance) {
		auto const numSamplesToDrop = std::min<int64_t>(fifoSamplesToRead(), -relativeSamplePosition);
		Log::msg(Log::Warning, "[SDLAudio render] must drop fifo data (%s ms)", numSamplesToDrop * 1000.0f / pcmFormat->sampleRate);
		fifoConsumeSamples((size_t)numSamplesToDrop);
	}

	if (relativeSamplePosition > audioJitterTolerance) {
		auto const numSilenceSamples = std::min<int64_t>(numSamplesToProduce, relativeSamplePosition);
		Log::msg(Log::Warning, "[SDLAudio render] insert silence (%s ms)", numSilenceSamples * 1000.0f / pcmFormat->sampleRate);
		silenceSamples(stream, (size_t)numSilenceSamples);
		numSamplesToProduce -= numSilenceSamples;
	}

	auto const numSamplesToConsume = std::min<int64_t>(numSamplesToProduce, fifoSamplesToRead());
	if (numSamplesToConsume > 0) {
		writeSamples(stream, m_Fifo.readPointer(), (size_t)numSamplesToConsume);
		fifoConsumeSamples((size_t)numSamplesToConsume);
		numSamplesToProduce -= numSamplesToConsume;
	}

	if (numSamplesToProduce > 0) {
		Log::msg(Log::Warning, "[SDLAudio render] underflow");
		silenceSamples(stream, (size_t)numSamplesToProduce);
	}
}

void SDLAudio::staticFillAudio(void *udata, uint8_t *stream, int len) {
	auto pThis = (SDLAudio*)udata;
	pThis->fillAudio(stream, len);
}

}
}
