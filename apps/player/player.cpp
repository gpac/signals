#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "../utils/tools.hpp"
#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

using namespace Tests;
using namespace Modules;

int safeMain(int argc, char const* argv[]) {

	if(argc != 2)
		throw std::runtime_error("usage: player <file>");

	auto const inputFile = argv[1];
	
	{
		auto demux = uptr(Demux::LibavDemux::create(inputFile));
		auto null = uptr(Out::Null::create());

		int videoIndex = -1;
		int audioIndex = -1;

		for (size_t i = 0; i < demux->getNumPin(); ++i) {
			Props *props = demux->getPin(i)->getProps();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) {
				videoIndex = i;
			} else if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_AUDIO) {
				audioIndex = i;
			} else {
				ConnectPin(demux->getPin(i), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
			}
		}

		if(videoIndex == -1)
			throw std::runtime_error("Video stream not found in file");

		if(audioIndex == -1)
			throw std::runtime_error("Audio stream not found in file");

		Props *videoProps = demux->getPin(videoIndex)->getProps();
		PropsDecoder *videoDecoderProps = dynamic_cast<PropsDecoder*>(videoProps);
		auto videoDecoder = uptr(Decode::LibavDecode::create(*videoDecoderProps));
		auto videoRenderer = uptr(Render::SDLVideo::create());
		ConnectPin(demux->getPin(videoIndex), videoDecoder.get(), &Decode::LibavDecode::process);
		ConnectPin(videoDecoder->getPin(0), videoRenderer.get(), &Render::SDLVideo::process);

		Props *audioProps = demux->getPin(audioIndex)->getProps();
		PropsDecoder *audioDecoderProps = dynamic_cast<PropsDecoder*>(audioProps);
		auto audioDecoder = uptr(Decode::LibavDecode::create(*audioDecoderProps));
		auto audioRenderer = uptr(Render::SDLAudio::create());

		ConnectPin(demux->getPin(audioIndex), audioDecoder.get(), &Decode::LibavDecode::process);
		ConnectPin(audioDecoder->getPin(0), audioRenderer.get(), &Render::SDLAudio::process);

		while (demux->process(nullptr)) {
		}

		demux->waitForCompletion();
		videoDecoder->waitForCompletion();
		audioDecoder->waitForCompletion();
	}

	return 0;
}

int main(int argc, char const* argv[]) {
	try {
		return safeMain(argc, argv);
	}
	catch(std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}


