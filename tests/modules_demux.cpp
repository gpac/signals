#include "tests.hpp"
#include "modules.hpp"
#include <memory>

using namespace Tests;
using namespace Modules;

namespace {

unittest("demux one track: Demux::GPACDemuxMP4Simple -> Out::Print")
{
	std::unique_ptr<Demux::GPACDemuxMP4Simple> mp4Demux(Demux::GPACDemuxMP4Simple::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));

	Connect(mp4Demux->getPin(0)->getSignal(), p.get(), &Out::Print::process);

	while (mp4Demux->process(nullptr)) {
	}
	mp4Demux->waitForCompletion();
}

unittest("demux one track: File -> Demux::GPACDemuxMP4Full -> Out::Print")
{
	std::unique_ptr<In::File> f(In::File::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	std::unique_ptr<Demux::GPACDemuxMP4Full> mp4Demux(Demux::GPACDemuxMP4Full::create());
	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));

	Connect(f->getPin(0)->getSignal(), mp4Demux.get(), &Demux::GPACDemuxMP4Full::process);
	Connect(mp4Demux->getPin(0)->getSignal(), p.get(), &Out::Print::process);

	while (f->process(nullptr)) {
	}

	f->waitForCompletion();
	mp4Demux->waitForCompletion();
}

}

