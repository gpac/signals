#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "src/in/sound_generator.hpp"
#include "../utils/tools.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("sound generator") {
	auto soundGen = uptr(new In::SoundGenerator);
	auto render = uptr(Render::SDLAudio::create());

	ConnectPin(soundGen->getPin(0), render.get(), &Render::SDLAudio::process);

	for(int i=0;i < 200;++i) {
		soundGen->process(nullptr);
	}

	soundGen->waitForCompletion();
}

}
