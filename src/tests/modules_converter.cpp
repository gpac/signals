#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/in/sound_generator.hpp"
#include "lib_media/transform/audio_convert.hpp"
#include "lib_media/transform/video_convert.hpp"
#include "lib_media/utils/comparator.hpp"
#include "lib_media/utils/recorder.hpp"
#include "lib_utils/tools.hpp"

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

unittest("audio converter: dynamic formats") {
	auto soundGen = uptr(new In::SoundGenerator);
	auto recorder = uptr(new Utils::Recorder);

	PcmFormat format;;
	auto converter = uptr(new Transform::AudioConvert(format));

	ConnectPinToModule(soundGen->getPin(0), converter);
	ConnectPinToModule(converter->getPin(0), recorder);

	{
		bool thrown = false;
		try {
			soundGen->process(nullptr);
		}
		catch (std::exception const& e) {
			std::cerr << "Expected error: " << e.what() << std::endl;
			thrown = true;
		}
		ASSERT(!thrown);
	}

	{
		bool thrown = false;
		try {
			Tools::Profiler profilerGlobal("  Send to converter");
			soundGen->process(nullptr);
		}
		catch (std::exception const& e) {
			std::cerr << "Expected error: " << e.what() << std::endl;
			thrown = true;
		}
		ASSERT(!thrown);
	}

	converter->flush();
	converter->getPin(0)->getSignal().disconnect(0);
	recorder->process(nullptr);

	{
		bool thrown = false;
		try {
			Tools::Profiler profilerGlobal("  Passthru");
			std::shared_ptr<const Data> data;
			while (data = recorder->pop()) {
				converter->process(data);
			}
		}
		catch (std::exception const& e) {
			std::cerr << "Expected error: " << e.what() << std::endl;
			thrown = true;
		}
		ASSERT(!thrown);
	}
}

unittest("video converter: pass-through") {
	auto res = Resolution(16, 32);
	int numFrames = 0;

	auto onFrame = [&](std::shared_ptr<const Data> data) {
		auto pic = safe_cast<const Picture>(data);
		ASSERT(pic->getResolution() == res);
		numFrames++;
	};

	{
		auto convert = uptr(new Transform::VideoConvert(
					res, AV_PIX_FMT_YUV420P,
					res, AV_PIX_FMT_YUV420P
					));
		ConnectPin(convert->getPin(0), onFrame);

		std::shared_ptr<Data> pic = uptr(new Picture(res));
		convert->process(pic);
	}

	ASSERT_EQUALS(1, numFrames);
}

unittest("video converter: different sizes") {
	auto srcRes = Resolution(16, 32);
	auto dstRes = Resolution(24, 8);
	int numFrames = 0;

	auto onFrame = [&](std::shared_ptr<const Data> data) {
		auto pic = safe_cast<const Picture>(data);
		ASSERT(pic->getResolution() == dstRes);
		numFrames++;
	};

	{
		auto convert = uptr(new Transform::VideoConvert(
					srcRes, AV_PIX_FMT_YUV420P,
					dstRes, AV_PIX_FMT_YUV420P
					));
		ConnectPin(convert->getPin(0), onFrame);

		std::shared_ptr<Data> pic = uptr(new Picture(srcRes));
		convert->process(pic);
	}

	ASSERT_EQUALS(1, numFrames);
}

}
