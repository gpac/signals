#pragma once

#include "lib_modules/core/module.hpp"
#include "lib_modules/core/clock.hpp"
#include <mutex>
#include <thread>

struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

namespace Modules {
namespace Render {

class SDLVideo : public Module {
public:
	SDLVideo(IClock* clock = g_DefaultClock);
	~SDLVideo();
	void process(std::shared_ptr<const Data> data) override;

private:

	void doRender();
	bool processOneFrame(std::shared_ptr<const Data> data);
	void createTexture();

	IClock* const m_clock;

	SDL_Window* window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	std::unique_ptr<SDL_Rect> displayrect;
	Resolution resolution;

	int64_t m_NumFrames;

	Queue<std::shared_ptr<const Data>> m_dataQueue;
	std::thread workingThread;
};

}
}
