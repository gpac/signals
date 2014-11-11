#include "tests.hpp"
#include "modules.hpp"

#include "in/sound_generator.hpp"
#include "transform/audio_convert.hpp"
#include "utils/comparator.hpp"
#include "utils/recorder.hpp"
#include "tools.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("audio converter: interleaved to planar to interleaved") {
	auto soundGen = uptr(new In::SoundGenerator);
	auto comparator = uptr(new Utils::PcmComparator);
	ConnectPin(soundGen->getPin(0), comparator.get(), &Utils::PcmComparator::pushOriginal);

	auto baseFormat  = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
	auto otherFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Planar);

	auto converter1 = uptr(new Transform::AudioConvert(baseFormat, otherFormat));
	auto converter2 = uptr(new Transform::AudioConvert(otherFormat, baseFormat));

	ConnectPinToModule(soundGen->getPin(0), converter1);
	ConnectPinToModule(converter1->getPin(0), converter2);
	ConnectPin(converter2->getPin(0), comparator.get(), &Utils::PcmComparator::pushOther);

	soundGen->process(nullptr);
	SLEEP_IN_MS(200); // HACK: allow time for the data to reach the comparator ...
	bool thrown = false;
	try {
		comparator->process(nullptr);
	}
	catch (std::exception const& e) {
		std::cerr << "Expected error: " << e.what() << std::endl;
		thrown = true;
	}
	ASSERT(!thrown);
}

unittest("audio converter: 44100 to 48000") {
	auto soundGen = uptr(new In::SoundGenerator);
	auto comparator = uptr(new Utils::PcmComparator);
	ConnectPin(soundGen->getPin(0), comparator.get(), &Utils::PcmComparator::pushOriginal);

	auto baseFormat  = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
	auto otherFormat = PcmFormat(48000, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);

	auto converter1 = uptr(new Transform::AudioConvert(baseFormat, otherFormat));
	auto converter2 = uptr(new Transform::AudioConvert(otherFormat, baseFormat));

	ConnectPinToModule(soundGen->getPin(0), converter1);
	ConnectPinToModule(converter1->getPin(0), converter2);
	ConnectPin(converter2->getPin(0), comparator.get(), &Utils::PcmComparator::pushOther);

	soundGen->process(nullptr);
	SLEEP_IN_MS(200); // HACK: allow time for the data to reach the comparator ...
	bool thrown = false;
	try {
		comparator->process(nullptr);
	}
	catch (std::exception const& e) {
		std::cerr << "Expected error: " << e.what() << std::endl;
		thrown = true;
	}
	ASSERT(!thrown);
}

}
