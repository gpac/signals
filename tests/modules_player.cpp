#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

using namespace Tests;
using namespace Modules;

namespace {

	unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Video Only) -> Render::SDL2") {
		std::unique_ptr<Demux::LibavDemux> demux(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		ASSERT(demux != nullptr);
		std::unique_ptr<Out::Null> null(Out::Null::create());
		ASSERT(null != nullptr);

		size_t videoIndex = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < demux->getNumPin(); ++i) {
			Props *props = demux->getPin(i)->getProps();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
				videoIndex = i;
			} else {
				Connect(demux->getPin(i)->getSignal(), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
			}
		}
		ASSERT(videoIndex != std::numeric_limits<size_t>::max());
		Props *props = demux->getPin(videoIndex)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		std::unique_ptr<Decode::LibavDecode> decode(Decode::LibavDecode::create(*decoderProps));
		ASSERT(decode != nullptr);

		std::unique_ptr<Render::SDLVideo> render(Render::SDLVideo::create());
		ASSERT(render != nullptr);

		Connect(demux->getPin(videoIndex)->getSignal(), decode.get(), &Decode::LibavDecode::process);
		Connect(decode->getPin(0)->getSignal(), render.get(), &Render::SDLVideo::process);

		while (demux->process(nullptr)) {
		}

		demux->destroy();
		decode->destroy();
	}

	unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Audio Only) -> Render::SDL2") {
		std::unique_ptr<Demux::LibavDemux> demux(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		ASSERT(demux != nullptr);
		std::unique_ptr<Out::Null> null(Out::Null::create());
		ASSERT(null != nullptr);

		size_t videoIndex = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < demux->getNumPin(); ++i) {
			Props *props = demux->getPin(i)->getProps();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_AUDIO) { //TODO: expose it somewhere
				videoIndex = i;
			} else {
				Connect(demux->getPin(i)->getSignal(), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
			}
		}
		ASSERT(videoIndex != std::numeric_limits<size_t>::max());
		Props *props = demux->getPin(videoIndex)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		std::unique_ptr<Decode::LibavDecode> decode(Decode::LibavDecode::create(*decoderProps));
		ASSERT(decode != nullptr);

		std::unique_ptr<Render::SDLAudio> render(Render::SDLAudio::create());
		ASSERT(render != nullptr);

		Connect(demux->getPin(videoIndex)->getSignal(), decode.get(), &Decode::LibavDecode::process);
		Connect(decode->getPin(0)->getSignal(), render.get(), &Render::SDLAudio::process);

		while (demux->process(nullptr)) {
		}

		demux->destroy();
		decode->destroy();
	}

}
