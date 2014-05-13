#include "../utils/log.hpp"
#include "SDL2/SDL.h"

class SdlInit {
public:
	SdlInit() {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) == -1) {
			Log::msg(Log::Warning, "[SDLAudio render] Couldn't initialize: %s", SDL_GetError());
			throw std::runtime_error("Init failed");
		}
		if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
			Log::msg(Log::Warning, "[SDLVideo render] Couldn't initialize: %s", SDL_GetError());
			throw std::runtime_error("Init failed");
		}
	}

	~SdlInit() {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
};

static SdlInit const g_InitSdl;

