#include "sdl_video.hpp"
#include "../utils/log.hpp"
#include "SDL2/SDL.h"

namespace Modules {
namespace Render {

SDLVideo* SDLVideo::create() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
		Log::msg(Log::Warning, "[SDLVideo render] Couldn't initialize: %s", SDL_GetError());
		throw std::runtime_error("Init failed");
	}

	const int width = VIDEO_WIDTH;
	const int height = VIDEO_HEIGHT;
	SDL_Window *window = SDL_CreateWindow("Signals SDLVideo renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!window) {
		Log::msg(Log::Warning, "[SDLVideo render]Couldn't set create window: %s", SDL_GetError());
		throw std::runtime_error("Window creation failed");
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		Log::msg(Log::Warning, "[SDLVideo render]Couldn't set create renderer: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		throw std::runtime_error("Renderer creation failed");
	}

	Uint32 pixelFormat = SDL_PIXELFORMAT_IYUV; //FIXME hardcoded
	//SDL_Texture *texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_Texture *texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STATIC, width, height);
	if (!texture) {
		Log::msg(Log::Warning, "[SDLVideo render]Couldn't set create texture: %s", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		throw std::runtime_error("Texture creation failed");
	}

	return new SDLVideo(renderer, texture, width, height, pixelFormat);
}

SDLVideo::SDLVideo(SDL_Renderer *renderer, SDL_Texture *texture, int width, int height, unsigned /*pixelFormat*/)
	: renderer(renderer), texture(texture), displayrect(new SDL_Rect()), width(width), height(height) {
	SDL_EventState(SDL_KEYUP, SDL_IGNORE); //ignore key up events, they don't even get filtered

	displayrect->x = 0;
	displayrect->y = 0;
	displayrect->w = width;
	displayrect->h = height;
}

SDLVideo::~SDLVideo() {
	SDL_DestroyRenderer(renderer);
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

	// sanity check
	assert((int)data->size() >= width * height * 3 / 2);

	SDL_UpdateYUVTexture(texture, NULL, data->data(), width, data->data() + width*height, width / 2, data->data()+(width*height*5)/4, width / 2);
	SDL_RenderCopy(renderer, texture, NULL, displayrect.get());
	SDL_RenderPresent(renderer);

	// hack until we have timestamps
	SDL_Delay(1);

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
