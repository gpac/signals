#include "sdl.hpp"
#include "../utils/log.hpp"
#include "SDL2/SDL.h"

namespace Modules {
namespace Render {

SDL* SDL::create() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
		Log::msg(Log::Error, "[SDL render] Couldn't initialize: %s", SDL_GetError());
		return nullptr;
	}

	const int width = 1280; //FIXME hardcoded
	const int height = 720; //FIXME hardcoded
	SDL_Window *window = SDL_CreateWindow("Signals SDL renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!window) {
		Log::msg(Log::Error, "Couldn't set create window: %s", SDL_GetError());
		return nullptr;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		Log::msg(Log::Error, "Couldn't set create renderer: %s", SDL_GetError());
		return nullptr;
	}

	Uint32 pixelFormat = SDL_PIXELFORMAT_IYUV; //FIXME hardcoded
	//SDL_Texture *texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_Texture *texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STATIC, width, height);
	if (!texture) {
		Log::msg(Log::Error, "Couldn't set create texture: %s", SDL_GetError());
		return nullptr;
	}

	return new SDL(renderer, texture, width, height, pixelFormat);
}

SDL::SDL(SDL_Renderer *renderer, SDL_Texture *texture, int width, int height, unsigned pixelFormat)
: renderer(renderer), texture(texture), displayrect(new SDL_Rect()), width(width), height(height), pixelFormat(pixelFormat) {/* Ignore key up events, they don't even get filtered */
	SDL_EventState(SDL_KEYUP, SDL_IGNORE);

	displayrect->x = 0;
	displayrect->y = 0;
	displayrect->w = width;
	displayrect->h = height;
}

SDL::~SDL() {
	SDL_DestroyRenderer(renderer);
}

bool SDL::process(std::shared_ptr<Data> data) {
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
		case SDL_MOUSEBUTTONDOWN:
			displayrect->x = event.button.x - width / 2;
			displayrect->y = event.button.y - height / 2;
			break;
		case SDL_MOUSEMOTION:
			if (event.motion.state) {
				displayrect->x = event.motion.x - width / 2;
				displayrect->y = event.motion.y - height / 2;
			}
			break;
		case SDL_QUIT:
			return false;
		}
	}

#if 0
	SDL_UpdateTexture(texture, NULL, data->data(), width*SDL_BYTESPERPIXEL(pixelFormat));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, displayrect.get());
	SDL_RenderPresent(renderer);
#endif
	SDL_UpdateYUVTexture(texture, NULL, data->data(), width, data->data() + width*height, width / 2, data->data()+(width*height*5)/4, width / 2);
	SDL_RenderCopy(renderer, texture, NULL, displayrect.get());
	SDL_RenderPresent(renderer);

	return true;
}

bool SDL::handles(const std::string &url) {
	return SDL::canHandle(url);
}

bool SDL::canHandle(const std::string &url) {
	return true;
}

}
}
