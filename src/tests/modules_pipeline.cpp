#include "tests.hpp"
#include "lib_media/demux/libav_demux.hpp"
#include "lib_media/mux/gpac_mux_mp4.hpp"
#include "lib_media/out/null.hpp"
#include "lib_modules/utils/pipeline.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("Pipeline: empty") {
	{
		Pipeline p;
	}
	{
		Pipeline p;
		p.start();
	}
	{
		Pipeline p;
		p.waitForCompletion();
	}
	{
		Pipeline p;
		p.start();
		p.waitForCompletion();
	}
}

unittest("Pipeline: interrupted") {
	//TODO
}

unittest("Pipeline: connect while running") {
	//TODO
}

unittest("Pipeline: connect one input to one output") {
	Pipeline p;
	auto demux = p.addModule(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	ASSERT(demux->getNumOutputs() > 1);
	auto null = p.addModule(new Out::Null);
	ConnectModules(demux, 0, null, 0);
	p.start();
	p.waitForCompletion();
}

unittest("Pipeline: connect inputs to outputs") {
	Pipeline p;
	auto demux = p.addModule(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto muxer = p.addModule(new Mux::GPACMuxMP4("output"));
	for (int i = 0; i < (int)demux->getNumOutputs(); ++i) {
		ConnectModules(demux, i, muxer, i);
	}
	p.start();
	p.waitForCompletion();
}

unittest("Pipeline: connect incompatible i/o") {
	//TODO
	bool thrown = false;
	try {
		thrown = true; //TODO
	} catch (std::runtime_error const& /*e*/) {
		thrown = true;
	}
	ASSERT(thrown);
}

}