#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/in/sound_generator.hpp"
#include "lib_media/in/video_generator.hpp"
#include "lib_media/render/sdl_audio.hpp"
#include "lib_media/render/sdl_video.hpp"
#include "lib_utils/tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("sound generator") {
	auto soundGen = uptr(new In::SoundGenerator);
	auto render = uptr(new Render::SDLAudio());

	ConnectPinToModule(soundGen->getPin(0),  render);

	for(int i=0; i < 25; ++i) {
		soundGen->process(nullptr);
	}
}

unittest("video generator") {
	auto clock = uptr(createSystemClock());
	auto videoGen = uptr(new In::VideoGenerator());
	auto render = uptr(new Render::SDLVideo(clock.get()));

	std::vector<int> times;
	auto onFrame = [&](std::shared_ptr<const Data> data) {
		auto rawData = safe_cast<const RawData>(data);
		times.push_back((int)rawData->getTime());
		render->process(rawData);
	};

	Connect(videoGen->getPin(0)->getSignal(), onFrame);

	for(int i=0; i < 50; ++i) {
		videoGen->process(nullptr);
	}

	ASSERT(times == makeVector(0, 7200, 180000, 187200));
}

}
