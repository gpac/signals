#include "sdl_audio.hpp"
#include "../utils/log.hpp"
#include "SDL2/SDL.h"
#include <cstring>

namespace Modules {
namespace Render {

std::mutex SDLAudio::audioMutex;
uint32_t SDLAudio::audioLen = 0;
uint8_t *SDLAudio::audioPos = 0;
std::vector<uint8_t> SDLAudio::audioData;

SDLAudio* SDLAudio::create() {
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) == -1) {
		Log::msg(Log::Warning, "[SDLAudio render] Couldn't initialize: %s", SDL_GetError());
		throw std::runtime_error("Init failed");
	}
	
	SDL_AudioSpec audioSpec;
	audioSpec.freq = 44100;
	audioSpec.format = AUDIO_F32SYS;
	audioSpec.channels = 2;    /* 1 = mono, 2 = stereo */
	audioSpec.samples = 1024;  /* Good low-latency value for callback */
	audioSpec.callback = &SDLAudio::fillAudio;
	audioSpec.userdata = NULL;
	if (SDL_OpenAudio(&audioSpec, NULL) < 0) {
		Log::msg(Log::Warning, "[SDLAudio render] Couldn't open audio: %s", SDL_GetError());
		throw std::runtime_error("Audio output creation failed");
	}

	return new SDLAudio();
}

SDLAudio::SDLAudio() {
	SDL_PauseAudio(0);
}

SDLAudio::~SDLAudio() {
	int remaining;
	{
		std::lock_guard<std::mutex> lg(audioMutex);
		remaining = audioLen;
	}
	if (remaining > 0) {
		do {
			SDL_Delay(10);
			std::lock_guard<std::mutex> lg(audioMutex);
			remaining = audioLen;
		} while (remaining != 0);
	}
	SDL_CloseAudio();
}

bool SDLAudio::process(std::shared_ptr<Data> data) {
	auto pcmData = dynamic_cast<PcmData*>(data.get());
	if(!pcmData) {
		Log::msg(Log::Warning, "[SDLAudio render] invalid packet type");
		return false;
	}

	{
		std::lock_guard<std::mutex> lg(audioMutex);
		const size_t newAudioSize = audioLen + pcmData->size();
		if (audioData.size() < newAudioSize) {
			audioData.resize(newAudioSize);
		}
		audioPos = audioData.data();
		memcpy(audioPos + audioLen, pcmData->data(), pcmData->size());
		audioLen = (uint32_t)newAudioSize;
	}

	return true;
}

void SDLAudio::fillAudio(void *udata, uint8_t *stream, int len) {
	std::lock_guard<std::mutex> lg(audioMutex);
	if (audioLen == 0) { //only play if we have data left
		return;
	}

	len = (len > (int)audioLen ? (int)audioLen : len);
	SDL_MixAudio(stream, audioPos, len, SDL_MIX_MAXVOLUME);
	audioLen -= len;
	if (audioLen > 0) {
		memmove(audioPos, audioPos + len, audioLen);
	}
}

bool SDLAudio::handles(const std::string &url) {
	return SDLAudio::canHandle(url);
}

bool SDLAudio::canHandle(const std::string &/*url*/) {
	return true;
}

}
}
