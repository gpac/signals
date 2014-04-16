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

unittest("sound generator") {
	auto soundGen = uptr(new In::SoundGenerator);
	auto render = uptr(Render::SDLAudio::create());

	ConnectPin(soundGen->getPin(0), render.get(), &Render::SDLAudio::process);

	for(int i=0; i < 200; ++i) {
		soundGen->process(nullptr);
	}

	soundGen->waitForCompletion();
}

unittest("video generator") {
	auto videoGen = uptr(new In::VideoGenerator);
	auto render = uptr(Render::SDLVideo::create());

	ConnectPin(videoGen->getPin(0), render.get(), &Render::SDLVideo::process);

	for(int i=0; i < 50; ++i) {
		videoGen->process(nullptr);
	}

	videoGen->waitForCompletion();
}

}
