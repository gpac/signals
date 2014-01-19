#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

using namespace Tests;
using namespace Modules;

namespace {

	unittest("transcoder: video simple") {
		//create demux
		std::unique_ptr<Demux::LibavDemux> demux(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		ASSERT(demux != nullptr);

		//create stub output (for unused demuxer's outputs)
		std::unique_ptr<Out::Null> null(Out::Null::create());
		ASSERT(null != nullptr);

		//find video signal from demux
		size_t videoIndex = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < demux->signals.size(); ++i) {
			Props *props = demux->signals[i]->props.get();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
				videoIndex = i;
			} else {
				//FIXME: we have to set Print output to avoid asserts. Should be remove once the framework is more tested.
				Connect(demux->signals[i]->signal, null.get(), &Out::Null::process);
			}
		}
		ASSERT(videoIndex != std::numeric_limits<size_t>::max());

		//create the video decoder
		Props *props = demux->signals[videoIndex]->props.get();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		std::unique_ptr<Decode::LibavDecode> decode(Decode::LibavDecode::create(*decoderProps));
		ASSERT(decode != nullptr);

		//create the mux
		std::unique_ptr<Mux::LibavMux> mux(Mux::LibavMux::create("output"));
		ASSERT(mux != nullptr);

		//create the encoder
		props = mux->signals[0]->props.get();
		PropsMuxer *muxerProps = dynamic_cast<PropsMuxer*>(props);
		std::unique_ptr<Encode::LibavEncode> encode(Encode::LibavEncode::create(*muxerProps, Encode::LibavEncode::Video));
		ASSERT(encode != nullptr);

		Connect(demux.get()->signals[videoIndex]->signal, decode.get(), &Decode::LibavDecode::process);
		Connect(decode.get()->signals[0]->signal, encode.get(), &Encode::LibavEncode::process);
		Connect(encode.get()->signals[0]->signal, mux.get(), &Mux::LibavMux::process);

		while (demux->process(nullptr)) {
		}

		demux->destroy();
		decode->destroy();
		mux->destroy();
		encode->destroy();
	}

}
