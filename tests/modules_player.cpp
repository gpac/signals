#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "../utils/tools.hpp"
#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

using namespace Tests;
using namespace Modules;

namespace {

	unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Video Only) -> Render::SDL2") {
		auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		auto null = uptr(Out::Null::create());

		size_t videoIndex = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < demux->getNumPin(); ++i) {
			Props *props = demux->getPin(i)->getProps();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
				videoIndex = i;
			} else {
				ConnectPin(demux->getPin(i), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
			}
		}
		ASSERT(videoIndex != std::numeric_limits<size_t>::max());
		Props *props = demux->getPin(videoIndex)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
		auto render = uptr(Render::SDLVideo::create());

		ConnectPin(demux->getPin(videoIndex), decode.get(), &Decode::LibavDecode::process);
		ConnectPin(decode->getPin(0), render.get(), &Render::SDLVideo::process);

		while (demux->process(nullptr)) {
		}

		demux->waitForCompletion();
		decode->waitForCompletion();
	}

	unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Audio Only) -> Render::SDL2") {
		auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		auto null = uptr(Out::Null::create());

		size_t videoIndex = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < demux->getNumPin(); ++i) {
			Props *props = demux->getPin(i)->getProps();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_AUDIO) { //TODO: expose it somewhere
				videoIndex = i;
			} else {
				ConnectPin(demux->getPin(i), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
			}
		}
		ASSERT(videoIndex != std::numeric_limits<size_t>::max());
		Props *props = demux->getPin(videoIndex)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
		auto render = uptr(Render::SDLAudio::create());

		ConnectPin(demux->getPin(videoIndex), decode.get(), &Decode::LibavDecode::process);
		ConnectPin(decode->getPin(0), render.get(), &Render::SDLAudio::process);

		while (demux->process(nullptr)) {
		}

		demux->waitForCompletion();
		decode->waitForCompletion();
	}

}
