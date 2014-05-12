#pragma once

#include "internal/module.hpp"
#include <mutex>
#include <string>
#include <thread>

struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

namespace Modules {
namespace Render {

//FIXME: check it doesn't need to run in thread 0, like most render on Unix do because of X11...
class SDLVideo : public Module {
public:
	SDLVideo();
	~SDLVideo();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:

	void doRender();
	void processOneFrame(std::shared_ptr<Data> data);

	/* Video */
	SDL_Window* window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	std::unique_ptr<SDL_Rect> displayrect;
	int width, height;

	int64_t m_NumFrames;

	QueueThreadSafe<std::shared_ptr<Data>> m_dataQueue;
	std::thread workingThread;
};

}
}
