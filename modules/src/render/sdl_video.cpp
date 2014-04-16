#include "sdl_video.hpp"
#include "../utils/log.hpp"
#include "SDL2/SDL.h"
#include "internal/clock.hpp"

namespace Modules {
namespace Render {

SDLVideo* SDLVideo::create() {
	return new SDLVideo;
}

SDLVideo::SDLVideo()
	: displayrect(new SDL_Rect()) {

	if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
		Log::msg(Log::Warning, "[SDLVideo render] Couldn't initialize: %s", SDL_GetError());
		throw std::runtime_error("Init failed");
	}

	width = VIDEO_WIDTH;
	height = VIDEO_HEIGHT;
	window = SDL_CreateWindow("Signals SDLVideo renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!window) {
		Log::msg(Log::Warning, "[SDLVideo render]Couldn't set create window: %s", SDL_GetError());
		throw std::runtime_error("Window creation failed");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		Log::msg(Log::Warning, "[SDLVideo render]Couldn't set create renderer: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		throw std::runtime_error("Renderer creation failed");
	}

	Uint32 pixelFormat = SDL_PIXELFORMAT_IYUV; //FIXME hardcoded
	//SDL_Texture *texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, width, height);
	texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STATIC, width, height);
	if (!texture) {
		Log::msg(Log::Warning, "[SDLVideo render]Couldn't set create texture: %s", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		throw std::runtime_error("Texture creation failed");
	}

	SDL_EventState(SDL_KEYUP, SDL_IGNORE); //ignore key up events, they don't even get filtered

	displayrect->x = 0;
	displayrect->y = 0;
	displayrect->w = width;
	displayrect->h = height;

	m_NumFrames = 0;
}

SDLVideo::~SDLVideo() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

bool SDLVideo::process(std::shared_ptr<Data> data) {
	/* Loop, waiting for QUIT or RESIZE */
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				SDL_RenderSetViewport(renderer, NULL);
				//displayrect->w = width  = event.window.data1;
				//displayrect->h = height = event.window.data2;
			}
			break;
		case SDL_QUIT:
			exit(1);
			return false;
		}
	}

	// sanity check
	assert((int)data->size() >= width * height * 3 / 2);

	auto const now = g_DefaultClock->now();
	auto const timestamp = data->getTime(); // assume timestamps start at zero
	auto const delay = (Uint32)std::max<int64_t>(0, timestamp - now);
	auto const delayInMs = delay / 180LL;
	SDL_Delay((Uint32)delayInMs);

	SDL_UpdateYUVTexture(texture, NULL, data->data(), width, data->data() + width*height, width / 2, data->data()+(width*height*5)/4, width / 2);
	SDL_RenderCopy(renderer, texture, NULL, displayrect.get());
	SDL_RenderPresent(renderer);

	m_NumFrames++;

	return true;
}

bool SDLVideo::handles(const std::string &url) {
	return SDLVideo::canHandle(url);
}

bool SDLVideo::canHandle(const std::string &/*url*/) {
	return true;
}
}
}
