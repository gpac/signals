#include "tests.hpp"
#include "modules.hpp"
#include <memory>

using namespace Tests;
using namespace Modules;

namespace {

	unittest("demux one track: GPAC_MP4_Simple -> Print")
	{
		std::unique_ptr<GPAC_MP4_Simple> mp4Demux(GPAC_MP4_Simple::create("data/BatmanHD_1000kbit_mpeg.mp4"));
		ASSERT(mp4Demux != nullptr);

		std::unique_ptr<Print> p(Print::create(std::cout));
		ASSERT(p != nullptr);

		CONNECT(mp4Demux.get(), signals[0]->signal, p.get(), &Print::process);
		while (mp4Demux->process(nullptr)) {
		}
		mp4Demux->destroy();
	}

	unittest("demux one track: File -> GPAC_MP4_Full -> Print")
	{
		std::unique_ptr<File> f(File::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		ASSERT(f != nullptr);

		std::unique_ptr<GPAC_MP4_Full> mp4Demux(GPAC_MP4_Full::create());

		std::unique_ptr<Print> p(Print::create(std::cout));
		ASSERT(p != nullptr);

		CONNECT(f.get(), signals[0]->signal, mp4Demux.get(), &GPAC_MP4_Full::process);
		CONNECT(mp4Demux.get(), signals[0]->signal, p.get(), &Print::process);

		f->push();

		f->destroy();
		mp4Demux->destroy();
	}

#if 0
	//TODO
	unittest(DemuxTwo, "demux two tracks: MP4Simple -> Print");

	//TODO
	unittest(DemuxDynamic, "demux a dynamic number of tracks: MP4Simple -> Print");
#endif

}

