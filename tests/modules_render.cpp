#include "tests.hpp"
#include "modules.hpp"

#include "render/sdl_audio.hpp"
#include "render/sdl_video.hpp"
#include "in/sound_generator.hpp"
#include "in/video_generator.hpp"
#include "../utils/tools.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("A/V sync: one thread") {
	auto videoGen = uptr(new In::VideoGenerator);
	auto videoRender = uptr(new Render::SDLVideo);
	ConnectPin(videoGen->getPin(0), videoRender.get(), &Render::SDLVideo::process);

	auto soundGen = uptr(new In::SoundGenerator);
	auto soundRender = uptr(Render::SDLAudio::create());
	ConnectPin(soundGen->getPin(0), soundRender.get(), &Render::SDLAudio::process);

	for(int i=0; i < 25*5; ++i) {
		videoGen->process(nullptr);
		soundGen->process(nullptr);
	}

	videoGen->waitForCompletion();
	videoRender->waitForCompletion();
	soundGen->waitForCompletion();
	soundRender->waitForCompletion();
}

unittest("A/V sync: separate threads") {
	auto f = [&]() {
		auto videoGen = uptr(new In::VideoGenerator);
		auto videoRender = uptr(new Render::SDLVideo);
		ConnectPin(videoGen->getPin(0), videoRender.get(), &Render::SDLVideo::process);

		for(int i=0; i < 25*5; ++i) {
			videoGen->process(nullptr);
		}

		videoGen->waitForCompletion();
		videoRender->waitForCompletion();
	};
	auto g = [&]() {
		auto soundGen = uptr(new In::SoundGenerator);
		auto soundRender = uptr(Render::SDLAudio::create());
		ConnectPin(soundGen->getPin(0), soundRender.get(), &Render::SDLAudio::process);
		for(int i=0; i < 25*5; ++i) {
			soundGen->process(nullptr);
		}

		soundGen->waitForCompletion();
		soundRender->waitForCompletion();
	};

	std::thread tf(f);
	std::thread tg(g);
	tf.join();
	tg.join();
}

}
