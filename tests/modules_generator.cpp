#include "tests.hpp"
#include "modules.hpp"

#include "in/sound_generator.hpp"
#include "in/video_generator.hpp"
#include "render/sdl_audio.hpp"
#include "render/sdl_video.hpp"
#include "transform/audio_convert.hpp"
#include "tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("sound generator") {
	auto soundGen = uptr(new In::SoundGenerator);
	auto render = uptr(Render::SDLAudio::create());
	auto converter = uptr(new Transform::AudioConvert(AudioSampleFormat::S16, AudioLayout::Stereo, 44100, AudioStruct::Interleaved,
		AudioSampleFormat::S16, AudioLayout::Stereo, 44100, AudioStruct::Planar));

	ConnectPinToModule(soundGen->getPin(0), converter);
	ConnectPinToModule(converter->getPin(0), render);

	for(int i=0; i < 25; ++i) {
		soundGen->process(nullptr);
	}
}

unittest("video generator") {
	auto videoGen = uptr(new In::VideoGenerator);
	auto render = uptr(new Render::SDLVideo);

	std::vector<int> times;
	auto onFrame = [&](std::shared_ptr<Data> data) {
		auto rawData = safe_cast<RawData>(data);
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
