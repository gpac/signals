#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/demux/libav_demux.hpp"
#include "lib_media/mux/gpac_mux_mp4.hpp"
#include "lib_media/mux/libav_mux.hpp"
#include "lib_utils/tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

#ifdef ENABLE_FAILING_TESTS
//ffmpeg extradata seems to be different (non annex B ?) when output from the muxer
unittest("remux test: GPAC mp4 mux") {
	auto demux = uptr(new Demux::LibavDemux("data/beepbop.mp4"));
	auto mux = uptr(new Mux::GPACMuxMP4("output_video_libav"));
	for (size_t i = 0; i < demux->getNumOutputs(); ++i) {
		ConnectModules(demux.get(), i, mux.get(), i);
		break; //FIXME
	}

	demux->process(nullptr);
}

unittest("remux test: libav mp4 mux") {
	auto demux = uptr(new Demux::LibavDemux("data/beepbop.mp4"));
	auto mux = uptr(new Mux::LibavMux("output_video_libav"));
	for (size_t i = 0; i < demux->getNumOutputs(); ++i) {
		ConnectModules(demux.get(), i, mux.get(), i);
		break; //FIXME
	}

	demux->process(nullptr);
}

unittest("multiple inputs: send same packets to 2 GPAC mp4 mux inputs") {
	//TODO
}
#endif

}
