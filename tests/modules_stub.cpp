#include "tests.hpp"

#include "../modules/src/demux/gpac_demux_mp4_simple.hpp"
#include "../modules/src/mux/gpac_mux_mp4.hpp"
#include "../modules/internal/utils/helper.hpp"

#include "tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("stub: gpac mp4 remux: GPAC demux -> GPAC mux") {
	auto demux = uptr(Demux::GPACDemuxMP4Simple::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	//find video signal from demux
#if 0
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
			videoIndex = i;
		}
		else {
			ConnectPinToModule(demux->getPin(i), null); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());
#endif

#if 0
	auto props = demux->getPin(videoIndex)->getProps();
#endif
	auto mux = uptr(Mux::GPACMuxMP4::create("output_video_gpac"));

#if 0
	//pass meta data between encoder and mux
	//Connect(demux->declareStream, mux.get(), &Mux::GPACMuxMP4::declareStream);
	//demux->sendOutputPinsInfo();

	ConnectPinToModule(demux->getPin(videoIndex), mux);

	while (demux->process(nullptr)) {
	}
#else
	//mitigate warnings
	demux->getNumPin();
	mux->getNumPin();
#endif
}

}
