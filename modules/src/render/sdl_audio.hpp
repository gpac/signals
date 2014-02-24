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
	static void staticFillAudio(void *udata, uint8_t *stream, int len);
	void fillAudio(uint8_t *stream, int len);

	std::mutex m_Mutex;
	uint32_t m_audioLen;
	uint8_t *m_audioPos;
	std::vector<uint8_t> m_audioData;
};

}
}
