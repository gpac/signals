#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include <memory>
#include <string>

struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;

namespace Modules {
namespace Render {

//FIXME: check it doesn't need to run in thread 0, like most render on Unix do because of X11...
class MODULES_EXPORT SDL : public Module {
public:
	static SDL* create();
	~SDL();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	SDL(SDL_Renderer *renderer, SDL_Texture *texture, int width, int height, unsigned pixelFormat);

	SDL_Renderer *renderer;
	SDL_Texture *texture;
	std::unique_ptr<SDL_Rect> displayrect;
	int width, height;
	const unsigned pixelFormat;
};

}
}
