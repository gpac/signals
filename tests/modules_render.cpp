#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "render/sdl_video.hpp"
#include "in/sound_generator.hpp"
#include "in/video_generator.hpp"
#include "../utils/tools.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("A/V sync") {

	auto soundGen = uptr(new In::SoundGenerator);
	auto soundRender = uptr(Render::SDLAudio::create());
	ConnectPin(soundGen->getPin(0), soundRender.get(), &Render::SDLAudio::process);

	auto videoGen = uptr(new In::VideoGenerator);
	auto videoRender = uptr(Render::SDLVideo::create());
	ConnectPin(videoGen->getPin(0), videoRender.get(), &Render::SDLVideo::process);

	for(int i=0;i < 4000;++i) {
		soundGen->process(nullptr);
	}

	for(int i=0;i < 70;++i) {
		videoGen->process(nullptr);
	}

	videoGen->waitForCompletion();
	soundGen->waitForCompletion();
}

}
