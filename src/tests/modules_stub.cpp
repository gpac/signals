#include "tests.hpp"

#include "../modules/src/demux/gpac_demux_mp4_simple.hpp"
#include "../modules/src/mux/gpac_mux_mp4.hpp"
#include "../modules/src/out/null.hpp"
#include "../modules/internal/utils/helper.hpp"

#include "tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("stub: gpac mp4 remux: GPAC demux -> GPAC mux") {
	auto demux = uptr(Demux::GPACDemuxMP4Simple::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(Out::Null::create());

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
#if 0
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
#else
		if (i == 0) {
#endif
			videoIndex = i;
		} else {
			ConnectPinToModule(demux->getPin(i), null); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	auto props = demux->getPin(videoIndex)->getProps();
	auto mux = uptr(new Mux::GPACMuxMP4("output_video_gpac"));

	//pass meta data between encoder and mux
#if 0
	Connect(demux->declareStream, mux.get(), &Mux::GPACMuxMP4::declareStream);
	demux->sendOutputPinsInfo();
#endif

	ConnectPinToModule(demux->getPin(videoIndex), mux);

	demux->process(nullptr);
}

}
