#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include <memory>
#include <mutex>
#include <string>

struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;

namespace Modules {
namespace Render {

//FIXME: check it doesn't need to run in thread 0, like most render on Unix do because of X11...
class MODULES_EXPORT SDLAudio : public Module {
public:
	static SDLAudio* create();
	~SDLAudio();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	SDLAudio();
	static void fillAudio(void *udata, uint8_t *stream, int len);

	static std::mutex audioMutex;
	static uint32_t audioLen;
	static uint8_t *audioPos;
	static std::vector<uint8_t> audioData;
};

}
}
