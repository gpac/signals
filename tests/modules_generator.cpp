#include "tests.hpp"
#include "modules.hpp"

#include "in/sound_generator.hpp"
#include "in/video_generator.hpp"
#include "render/sdl_audio.hpp"
#include "render/sdl_video.hpp"
#include "../utils/tools.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("sound generator") {
	auto soundGen = uptr(new In::SoundGenerator);
	auto render = uptr(Render::SDLAudio::create());

	ConnectPin(soundGen->getPin(0), render.get(), &Render::SDLAudio::process);

	for(int i=0; i < 25; ++i) {
		soundGen->process(nullptr);
	}

	soundGen->waitForCompletion();
}

unittest("video generator") {
	auto videoGen = uptr(new In::VideoGenerator);
	auto render = uptr(new Render::SDLVideo);

	std::vector<int> times;
	auto onFrame = [&](std::shared_ptr<Data> data) -> bool {
		times.push_back((int)data->getTime());
		render->process(data);
		return true;
	};

	Connect(videoGen->getPin(0)->getSignal(), onFrame);

	for(int i=0; i < 50; ++i) {
		videoGen->process(nullptr);
	}

	videoGen->waitForCompletion();
	render->waitForCompletion();

	ASSERT(times == makeVector(0, 7200, 180000, 187200));
}

}
