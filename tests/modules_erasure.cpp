#include "tests.hpp"
#include "modules.hpp"
#include <memory>

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

}
