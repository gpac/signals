#include "tests.hpp"
#include "modules.hpp"
#include "../utils/tools.hpp"

#include "decode/libav_decode.hpp"
#include "demux/libav_demux.hpp"
#include "out/print.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("Packet type erasure + multi-output-pin: libav Demux -> {libav Decoder -> Out::Print}*") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	ASSERT(demux != nullptr);

	std::vector<std::unique_ptr<Decode::LibavDecode>> decoders;
	std::vector<std::unique_ptr<Out::Print>> printers;
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
		ASSERT(decode != nullptr);

		auto p = uptr(Out::Print::create(std::cout));
		ASSERT(p != nullptr);

		ConnectPinToModule(demux->getPin(i), decode);
		ConnectPinToModule(decode->getPin(0), p);

		decoders.push_back(std::move(decode));
		printers.push_back(std::move(p));
	}

	while (demux->process(nullptr)) {
	}

	demux->waitForCompletion();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		decoders[i]->waitForCompletion();
	}
}

}
