#include "tests.hpp"
#include "modules.hpp"
#include <memory>

using namespace Tests;
using namespace Modules;

namespace {

unittest("demux one track: Demux::GPAC_MP4_Simple -> Out::Print")
{
	std::unique_ptr<Demux::GPAC_MP4_Simple> mp4Demux(Demux::GPAC_MP4_Simple::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));

	Connect(mp4Demux->getSignal(0), p.get(), &Out::Print::process);

	while (mp4Demux->process(nullptr)) {
	}
	mp4Demux->destroy();
}

unittest("demux one track: File -> Demux::GPAC_MP4_Full -> Out::Print")
{
	std::unique_ptr<In::File> f(In::File::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	std::unique_ptr<Demux::GPAC_MP4_Full> mp4Demux(Demux::GPAC_MP4_Full::create());
	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));

	Connect(f->getSignal(0), mp4Demux.get(), &Demux::GPAC_MP4_Full::process);
	Connect(mp4Demux->getSignal(0), p.get(), &Out::Print::process);

	f->push();

	f->destroy();
	mp4Demux->destroy();
}

}

