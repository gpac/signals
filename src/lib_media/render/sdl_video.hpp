#pragma once

#include "../common/picture.hpp"
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

class SDLVideo : public ModuleS {
	public:
		SDLVideo(IClock* clock = g_DefaultClock);
		~SDLVideo();
		void process(Data data) override;

	private:

		void doRender();
		bool processOneFrame(Data data);
		void createTexture();

		IClock* const m_clock;

		SDL_Window* window;
		SDL_Renderer *renderer;
		SDL_Texture *texture;
		std::unique_ptr<SDL_Rect> displayrect;
		PictureFormat pictureFormat;

		int64_t m_NumFrames;

		Signals::Queue<Data> m_dataQueue; //FIXME: useless now we have input pins
		std::thread workingThread;
};

}
}
