#include "sdl_video.hpp"
#include "lib_utils/tools.hpp"
#include "SDL2/SDL.h"
#include "render_common.hpp"

namespace Modules {
namespace Render {

namespace {
Uint32 pixelFormat2SDLFormat(const Modules::PixelFormat format) {
	switch (format) {
	case YUV420P: return SDL_PIXELFORMAT_IYUV;
	case YUYV422: return SDL_PIXELFORMAT_YUY2;
	case RGB24: return SDL_PIXELFORMAT_RGB24;
	default: throw std::runtime_error("Pixel format not supported.");
	}
}
}

SDLVideo::SDLVideo(IClock* clock)
	: m_clock(clock), texture(nullptr), displayrect(new SDL_Rect()), workingThread(&SDLVideo::doRender, this) {
	auto input = addInput(new Input<DataPicture>(this));
	input->setMetadata(new MetadataRawVideo);
	m_dataQueue.pop();
}

void SDLVideo::doRender() {
	pictureFormat.res = VIDEO_RESOLUTION;
	pictureFormat.format = YUV420P;
	window = SDL_CreateWindow("Signals SDLVideo renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pictureFormat.res.width, pictureFormat.res.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!window)
		throw error(format("Couldn't set create window: %s", SDL_GetError()));

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		SDL_DestroyWindow(window);
		throw error(format("Couldn't set create renderer: %s", SDL_GetError()));
	}
	m_dataQueue.push(nullptr); //unlock the constructor

	createTexture();

	SDL_EventState(SDL_KEYUP, SDL_IGNORE); //ignore key up events, they don't even get filtered

	m_NumFrames = 0;

	for(;;) {
		auto data = m_dataQueue.pop();
		if(!data)
			break;
		if (!processOneFrame(data))
			break;
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

bool SDLVideo::processOneFrame(Data data) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				SDL_RenderSetViewport(renderer, nullptr);
				displayrect->w = event.window.data1;
				displayrect->h = event.window.data2;
			}
			break;
		case SDL_QUIT:
			exit(1);
			return false;
		}
	}

	auto pic = safe_cast<const DataPicture>(data);
	if (pic->getFormat() != pictureFormat) {
		pictureFormat = pic->getFormat();
		createTexture();
	}

	auto const now = m_clock->now();
	auto const timestamp = pic->getTime() + PREROLL_DELAY; // assume timestamps start at zero
	auto const delay = std::max<int64_t>(0, timestamp - now);
	auto const delayInMs = clockToTimescale(delay, 1000);
	SDL_Delay((Uint32)delayInMs);

	if (pictureFormat.format == YUV420P) {
		SDL_UpdateYUVTexture(texture, nullptr,
		                     pic->getPlane(0), (int)pic->getPitch(0),
		                     pic->getPlane(1), (int)pic->getPitch(1),
		                     pic->getPlane(2), (int)pic->getPitch(2));
	} else {
		SDL_UpdateTexture(texture, nullptr, pic->getPlane(0), (int)pic->getPitch(0));
	}
	SDL_RenderCopy(renderer, texture, nullptr, displayrect.get());
	SDL_RenderPresent(renderer);

	m_NumFrames++;

	return true;
}

void SDLVideo::createTexture() {
	log(Info, format("%sx%s", pictureFormat.res.width, pictureFormat.res.height));

	if (texture)
		SDL_DestroyTexture(texture);

	texture = SDL_CreateTexture(renderer, pixelFormat2SDLFormat(pictureFormat.format), SDL_TEXTUREACCESS_STATIC, pictureFormat.res.width, pictureFormat.res.height);
	if (!texture)
		throw error(format("Couldn't set create texture: %s", SDL_GetError()));

	displayrect->x = 0;
	displayrect->y = 0;
	displayrect->w = pictureFormat.res.width;
	displayrect->h = pictureFormat.res.height;
	SDL_SetWindowSize(window, displayrect->w, displayrect->h);
}

SDLVideo::~SDLVideo() {
	m_dataQueue.push(nullptr);
	workingThread.join();
}

void SDLVideo::process(Data data) {
	m_dataQueue.push(data);
}

}
}
