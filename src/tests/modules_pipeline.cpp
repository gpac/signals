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

#ifdef ENABLE_FAILING_TESTS
unittest("Pipeline: interrupted") {
	//TODO
}

unittest("Pipeline: connect while running") {
	//TODO
}
#endif

unittest("Pipeline: connect one input (out of 2) to one output") {
	Pipeline p;
	auto demux = p.addModule(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(demux->getNumOutputs() > 1);
	auto null = p.addModule(new Out::Null);
	p.connect(demux, 0, null, 0);
	p.start();
	p.waitForCompletion();
}

#ifdef ENABLE_FAILING_TESTS
unittest("Pipeline: connect inputs to outputs") {
	Pipeline p;
	auto demux = p.addModule(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto muxer = p.addModule(new Mux::GPACMuxMP4("output"));
	for (int i = 0; i < (int)demux->getNumOutputs(); ++i) {
		p.connect(demux, i, muxer, i);
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
#endif

}