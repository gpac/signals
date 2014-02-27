#include "tests.hpp"
#include "modules.hpp"
#include "../utils/tools.hpp"
#include <memory>

using namespace Tests;
using namespace Modules;

namespace {

unittest("demux one track: Demux::GPACDemuxMP4Simple -> Out::Print") {
	auto mp4Demux = uptr(Demux::GPACDemuxMP4Simple::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto p = uptr(Out::Print::create(std::cout));

	ConnectPin(mp4Demux->getPin(0), p.get(), &Out::Print::process);

	while (mp4Demux->process(nullptr)) {
	}
	mp4Demux->waitForCompletion();
}

unittest("demux one track: File -> Demux::GPACDemuxMP4Full -> Out::Print") {
	auto f = uptr(In::File::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto mp4Demux = uptr(Demux::GPACDemuxMP4Full::create());
	auto p = uptr(Out::Print::create(std::cout));

	ConnectPin(f->getPin(0), mp4Demux.get(), &Demux::GPACDemuxMP4Full::process);
	ConnectPin(mp4Demux->getPin(0), p.get(), &Out::Print::process);

	while (f->process(nullptr)) {
	}

	f->waitForCompletion();
	mp4Demux->waitForCompletion();
}

}

