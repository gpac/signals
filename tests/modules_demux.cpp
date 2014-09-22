#include "tests.hpp"
#include "modules.hpp"
#include "../utils/tools.hpp"

#include "demux/gpac_demux_mp4_simple.hpp"
#include "demux/gpac_demux_mp4_full.hpp"
#include "in/file.hpp"
#include "out/print.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("demux one track: Demux::GPACDemuxMP4Simple -> Out::Print") {
	auto mp4Demux = uptr(Demux::GPACDemuxMP4Simple::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto p = uptr(Out::Print::create(std::cout));

	ConnectPinToModule(mp4Demux->getPin(0), p.get());

	while (mp4Demux->process(nullptr)) {
	}
	mp4Demux->waitForCompletion();
}

unittest("demux one track: File -> Demux::GPACDemuxMP4Full -> Out::Print") {
	auto f = uptr(In::File::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto mp4Demux = uptr(Demux::GPACDemuxMP4Full::create());
	auto p = uptr(Out::Print::create(std::cout));

	ConnectPinToModule(f->getPin(0), mp4Demux.get());
	ConnectPinToModule(mp4Demux->getPin(0), p.get());

	f->process(nullptr);

	f->waitForCompletion();
	mp4Demux->waitForCompletion();
}

}

