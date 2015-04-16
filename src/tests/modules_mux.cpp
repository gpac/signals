#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/demux/libav_demux.hpp"
#include "lib_media/mux/gpac_mux_mp4.hpp"
#include "lib_utils/tools.hpp"


using namespace Tests;
using namespace Modules;

#ifdef ENABLE_FAILING_TESTS
unittest("remux test: GPAC mp4 mux") {
	auto demux = uptr(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto mux = uptr(new Mux::GPACMuxMP4("output_video_libav"));
	for (size_t i = 0; i < demux->getNumOutputPins(); ++i) {
		//auto props = demux->getOutputPin(i)->getProps();
		//auto decoderProps = safe_cast<PropsPkt>(props);
		//TODO: rename helpers? Pin/Module -> Connector/Connected ?
		ConnectPinToModule(demux->getOutputPin(i), mux->getInputPin(i));
	}
	//encode->sendOutputPinsInfo();

	demux->process(nullptr);
}
#endif
