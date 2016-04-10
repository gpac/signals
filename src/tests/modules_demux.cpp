#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/demux/gpac_demux_mp4_simple.hpp"
#include "lib_media/demux/gpac_demux_mp4_full.hpp"
#include "lib_media/in/file.hpp"
#include "lib_media/out/print.hpp"
#include "lib_utils/tools.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("demux one track: Demux::GPACDemuxMP4Simple -> Out::Print") {
	auto mp4Demux = uptr(new Demux::GPACDemuxMP4Simple("data/beepbop.mp4"));
	auto p = uptr(new Out::Print(std::cout));

	ConnectOutputToInput(mp4Demux->getOutput(0), p);

	mp4Demux->process(nullptr);
}

unittest("demux one track: File -> Demux::GPACDemuxMP4Full -> Out::Print") {
	auto f = uptr(new In::File("data/beepbop.mp4"));
	auto mp4Demux = uptr(new Demux::GPACDemuxMP4Full());
	auto p = uptr(new Out::Print(std::cout));

	ConnectOutputToInput(f->getOutput(0), mp4Demux);
	ConnectOutputToInput(mp4Demux->getOutput(0), p);

	f->process(nullptr);
}

}

