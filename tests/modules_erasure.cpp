#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

using namespace Tests;
using namespace Modules;

namespace {

	unittest("Packet type erasure + multi-output-pin: libav Demux -> {libav Decoder -> Out::Print}*") {
		std::unique_ptr<Demux::Libavformat_55> demux(Demux::Libavformat_55::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		ASSERT(demux != nullptr);

		std::vector<std::unique_ptr<Decode::Libavcodec_55>> decoders;
		std::vector<std::unique_ptr<Out::Print>> printers;
		for (size_t i = 0; i < demux->signals.size(); ++i) {
			//FIXME: const Props *props = demux->signals[i]->getProps();
			Props *props = demux->signals[i]->props.get();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			std::unique_ptr<Decode::Libavcodec_55> decode(Decode::Libavcodec_55::create(*decoderProps));
			ASSERT(decode != nullptr);

			std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));
			ASSERT(p != nullptr);

			Connect(demux->signals[i]->signal, decode.get(), &Decode::Libavcodec_55::process);
			Connect(decode->signals[0]->signal, p.get(), &Out::Print::process);

			decoders.push_back(std::move(decode));
			printers.push_back(std::move(p));
		}


		while (demux->process(nullptr)) {
		}

		demux->destroy();
		for (size_t i = 0; i < demux->signals.size(); ++i) {
			decoders[i]->destroy();
		}
	}

	unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Video Only) -> Render::SDL2") {
		std::unique_ptr<Demux::Libavformat_55> demux(Demux::Libavformat_55::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		ASSERT(demux != nullptr);
		std::unique_ptr<Out::Null> null(Out::Null::create());
		ASSERT(null != nullptr);

		size_t videoIndex = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < demux->signals.size(); ++i) {
			Props *props = demux->signals[i]->props.get();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
				videoIndex = i;
			} else {
				//FIXME: we have to set Print output to avoid asserts. Should be remove once the framework is more tested.
				Connect(demux->signals[i]->signal, null.get(), &Out::Null::process);
			}
		}
		ASSERT(videoIndex != std::numeric_limits<size_t>::max());
		Props *props = demux->signals[videoIndex]->props.get();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		std::unique_ptr<Decode::Libavcodec_55> decode(Decode::Libavcodec_55::create(*decoderProps));
		ASSERT(decode != nullptr);

		std::unique_ptr<Render::SDL> render(Render::SDL::create());
		ASSERT(render != nullptr);

		Connect(demux->signals[videoIndex]->signal, decode.get(), &Decode::Libavcodec_55::process);
		Connect(decode->signals[0]->signal, render.get(), &Render::SDL::process);

		while (demux->process(nullptr)) {
		}

		demux->destroy();
		decode->destroy();
	}

}
