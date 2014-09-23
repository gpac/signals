#include "tests.hpp"
#include "modules.hpp"

#include "demux/gpac_demux_mp4_simple.hpp"
#include "demux/gpac_demux_mp4_full.hpp"
#include "in/file.hpp"
#include "out/print.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("demux one track: Demux::GPACDemuxMP4Simple -> Out::Print") {
	auto mp4Demux = uptrSafeModule(Demux::GPACDemuxMP4Simple::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto p = uptrSafeModule(new Out::Print(std::cout));

	ConnectPinToModule(mp4Demux->getPin(0), p);

	while (mp4Demux->process(nullptr)) {
	}
	mp4Demux->waitForCompletion();
}

unittest("demux one track: File -> Demux::GPACDemuxMP4Full -> Out::Print") {
	auto f = uptrSafeModule(In::File::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto mp4Demux = uptrSafeModule(Demux::GPACDemuxMP4Full::create());
	auto p = uptrSafeModule(new Out::Print(std::cout));

	ConnectPinToModule(f->getPin(0), mp4Demux);
	ConnectPinToModule(mp4Demux->getPin(0), p);

	f->process(nullptr);

	f->waitForCompletion();
	mp4Demux->waitForCompletion();
}

}

