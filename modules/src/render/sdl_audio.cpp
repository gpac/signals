#include "sdl_audio.hpp"
#include "../utils/log.hpp"
#include "SDL2/SDL.h"
#include <cstring>
#include <fstream>

namespace Modules {
namespace Render {

SDLAudio* SDLAudio::create() {
	return new SDLAudio();
}

SDLAudio::SDLAudio() {
	if (SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) == -1) {
		Log::msg(Log::Warning, "[SDLAudio render] Couldn't initialize: %s", SDL_GetError());
		throw std::runtime_error("Init failed");
	}

	SDL_AudioSpec audioSpec;
	audioSpec.freq = AUDIO_SAMPLERATE;
	audioSpec.format = AUDIO_S16;
	audioSpec.channels = 2;    /* 1 = mono, 2 = stereo */
	audioSpec.samples = 1024;  /* Good low-latency value for callback */
	audioSpec.callback = &SDLAudio::staticFillAudio;
	audioSpec.userdata = this;

	SDL_AudioSpec realSpec;

	if (SDL_OpenAudio(&audioSpec, &realSpec) < 0) {
		Log::msg(Log::Warning, "[SDLAudio render] Couldn't open audio: %s", SDL_GetError());
		throw std::runtime_error("Audio output creation failed");
	}

	auto const latencyInMs = float(realSpec.samples) * 1000.0f / realSpec.freq;
	Log::msg(Log::Info, "[SDLAudio render] %s Hz %s ms", realSpec.freq, latencyInMs);

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
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool SDLAudio::process(std::shared_ptr<Data> data) {
	auto pcmData = dynamic_cast<PcmData*>(data.get());
	if (!pcmData) {
		Log::msg(Log::Warning, "[SDLAudio render] invalid packet type");
		return false;
	}

	{
		std::lock_guard<std::mutex> lg(m_Mutex);
		m_Fifo.write(pcmData->data(), (size_t)pcmData->size());
	}

	return true;
}

void SDLAudio::fillAudio(uint8_t *stream, int len) {
	std::lock_guard<std::mutex> lg(m_Mutex);

	if (len > (int)m_Fifo.bytesToRead()) {
		Log::msg(Log::Warning, "[SDLAudio render] underflow");
		memset(stream, 0, len);
		len = (int)m_Fifo.bytesToRead();
	}

	if (len > 0) {
		memcpy(stream, m_Fifo.readPointer(), len);
		m_Fifo.consume(len);
	}
}

void SDLAudio::staticFillAudio(void *udata, uint8_t *stream, int len) {
	auto pThis = (SDLAudio*)udata;
	pThis->fillAudio(stream, len);
}

bool SDLAudio::handles(const std::string &url) {
	return SDLAudio::canHandle(url);
}

bool SDLAudio::canHandle(const std::string &/*url*/) {
	return true;
}

}
}
