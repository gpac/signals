#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/render/sdl_audio.hpp"
#include "lib_media/render/sdl_video.hpp"
#include "lib_media/in/sound_generator.hpp"
#include "lib_media/in/video_generator.hpp"
#include "lib_utils/tools.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("render: A/V sync, one thread") {
	auto clock = uptr(createSystemClock());
	auto videoGen = uptr(new In::VideoGenerator);
	auto videoRender = uptr(new Render::SDLVideo(clock.get()));
	ConnectPinToModule(videoGen->getPin(0), videoRender);

	//FIXME: avoid SDL audio and video parallel creations
	const int sleepDurInMs = 100;
	const std::chrono::milliseconds dur(sleepDurInMs);
	std::this_thread::sleep_for(dur);

	auto soundGen = uptr(new In::SoundGenerator);
	auto soundRender = uptr(Render::SDLAudio::create(clock.get()));
	ConnectPinToModule(soundGen->getPin(0), soundRender);

	for(int i=0; i < 25*5; ++i) {
		videoGen->process(nullptr);
		soundGen->process(nullptr);
	}
}

unittest("render: dynamic resolution") {
	auto videoRender = uptr(new Render::SDLVideo);

	std::shared_ptr<Data> pic1 = uptr(new Picture(Resolution(128, 64)));
	pic1->setTime(1000);
	videoRender->process(pic1);

	std::shared_ptr<Data> pic2 = uptr(new Picture(Resolution(64, 256)));
	pic2->setTime(2000);
	videoRender->process(pic2);
}

#ifdef __linux__
unittest("render: A/V sync: separate threads") {
	auto f = [&]() {
		auto videoGen = uptr(new In::VideoGenerator);
		auto videoRender = uptr(new Render::SDLVideo);
		ConnectPinToModule(videoGen->getPin(0), videoRender);

		for(int i=0; i < 25*5; ++i) {
			videoGen->process(nullptr);
		}
	};
	auto g = [&]() {
		auto soundGen = uptr(new In::SoundGenerator);
		auto soundRender = uptr(Render::SDLAudio::create());
		ConnectPinToModule(soundGen->getPin(0), soundRender);
		for(int i=0; i < 25*5; ++i) {
			soundGen->process(nullptr);
		}
	};

	std::thread tf(f);

	//FIXME: avoid SDL audio and video parallel creations
	const int sleepDurInMs = 100;
	const std::chrono::milliseconds dur(sleepDurInMs);
	std::this_thread::sleep_for(dur);

	std::thread tg(g);

	tf.join();
	tg.join();
}
#endif

}
