#include "sdl_video.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "SDL2/SDL.h"
#include "render_common.hpp"


namespace Modules {
namespace Render {

SDLVideo::SDLVideo(IClock* clock)
	: m_clock(clock), texture(nullptr), displayrect(new SDL_Rect()), workingThread(&SDLVideo::doRender, this) {
	m_dataQueue.pop();
}

void SDLVideo::doRender() {
	resolution = VIDEO_RESOLUTION;
	window = SDL_CreateWindow("Signals SDLVideo renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, resolution.width, resolution.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
	m_dataQueue.push(nullptr); //unlock the constructor

	createTexture();

	SDL_EventState(SDL_KEYUP, SDL_IGNORE); //ignore key up events, they don't even get filtered

	displayrect->x = 0;
	displayrect->y = 0;
	displayrect->w = resolution.width;
	displayrect->h = resolution.height;

	m_NumFrames = 0;

	for(;;) {
		auto data = m_dataQueue.pop();
		if(!data)
			break;
		processOneFrame(data);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void SDLVideo::processOneFrame(std::shared_ptr<const Data> data) {
	/* Loop, waiting for QUIT or RESIZE */
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				SDL_RenderSetViewport(renderer, NULL);
				displayrect->w = event.window.data1;
				displayrect->h = event.window.data2;
			}
			break;
		case SDL_QUIT:
			exit(1);
			return;
		}
	}

	// sanity check
	auto pic = safe_cast<const Picture>(data);

	auto const now = m_clock->now();
	auto const timestamp = pic->getTime() + PREROLL_DELAY; // assume timestamps start at zero
	auto const delay = (Uint32)std::max<int64_t>(0, timestamp - now);
	auto const delayInMs = clockToTimescale(delay, 1000);
	SDL_Delay((Uint32)delayInMs);

	if(pic->getResolution() != resolution) {
		resolution = pic->getResolution();
		createTexture();
	}

	SDL_UpdateYUVTexture(texture, NULL, 
			pic->getComp(0), (int)pic->getPitch(0),
			pic->getComp(1), (int)pic->getPitch(1),
			pic->getComp(2), (int)pic->getPitch(2));
	SDL_RenderCopy(renderer, texture, NULL, displayrect.get());
	SDL_RenderPresent(renderer);

	m_NumFrames++;
}

void SDLVideo::createTexture() {
	Log::msg(Log::Info, format("[SDLVideo render] %sx%s", resolution.width, resolution.height));

	if(texture)
		SDL_DestroyTexture(texture);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STATIC, resolution.width, resolution.height);
	if (!texture) {
		Log::msg(Log::Warning, "[SDLVideo render]Couldn't set create texture: %s", SDL_GetError());
		throw std::runtime_error("Texture creation failed");
	}
}

SDLVideo::~SDLVideo() {
	m_dataQueue.push(nullptr);
	workingThread.join();
}

void SDLVideo::process(std::shared_ptr<const Data> data) {
	m_dataQueue.push(data);
}

}
}
