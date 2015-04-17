#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/decode/libav_decode.hpp"
#include "lib_media/demux/libav_demux.hpp"
#include "lib_media/out/print.hpp"
#include "lib_utils/tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("Packet type erasure + multi-output-pin: libav Demux -> {libav Decoder -> Out::Print}*") {
	auto demux = uptr(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	std::vector<std::unique_ptr<Decode::LibavDecode>> decoders;
	std::vector<std::unique_ptr<Out::Print>> printers;
	for (size_t i = 0; i < demux->getNumOutputPins(); ++i) {
		auto metadata = safe_cast<Metadata>(demux->getOutputPin(i))->getMetadata();
		auto decode = uptr(new Decode::LibavDecode(*safe_cast<MetadataPktLibav>(metadata));

		auto p = uptr(new Out::Print(std::cout));

		ConnectPinToModule(demux->getOutputPin(i), decode);
		ConnectPinToModule(decode->getOutputPin(0), p);

		decoders.push_back(std::move(decode));
		printers.push_back(std::move(p));
	}

	demux->process(nullptr);
}

}
