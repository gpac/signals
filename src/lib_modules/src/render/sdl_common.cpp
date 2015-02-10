#include "lib_utils/log.hpp"
#include "SDL2/SDL.h"

#ifdef __linux__
#include <signal.h>
#endif

class SdlInit {
	public:
		SdlInit() {
#ifdef __linux__
			struct sigaction action;
			sigaction(SIGINT, nullptr, &action);
#endif
			if (SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) == -1) {
				Log::msg(Log::Warning, "[SDLAudio render] Couldn't initialize: %s", SDL_GetError());
				throw std::runtime_error("Init failed");
			}
			if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
				Log::msg(Log::Warning, "[SDLVideo render] Couldn't initialize: %s", SDL_GetError());
				throw std::runtime_error("Init failed");
			}
#ifdef __linux__
			sigaction(SIGINT, &action, nullptr);
#endif
		}

		~SdlInit() {
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
		}
};

static SdlInit const g_InitSdl;

