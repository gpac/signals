#include "tests.hpp"
#include "modules.hpp"

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

#include "decode/libav_decode.hpp"
#include "demux/libav_demux.hpp"
#include "encode/libav_encode.hpp"
#include "mux/libav_mux.hpp"
#include "mux/gpac_mux_mp4.hpp"
#include "out/null.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("transcoder: video simple (libav mux)") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(Out::Null::create());

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
			videoIndex = i;
		} else {
			ConnectPinToModule(demux->getPin(i), null); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	auto props = demux->getPin(videoIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);

	auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
	auto encode = uptr(Encode::LibavEncode::create(Encode::LibavEncode::Video));
	auto mux = uptr(Mux::LibavMux::create("output_video_libav"));

	//pass meta data between encoder and mux
	Connect(encode->declareStream, mux.get(), &Mux::LibavMux::declareStream);
	encode->sendOutputPinsInfo();

	ConnectPinToModule(demux->getPin(videoIndex), decode);
	ConnectPinToModule(decode->getPin(0), encode);
	ConnectPinToModule(encode->getPin(0), mux);

	demux->process(nullptr);
}

unittest("transcoder: video simple (gpac mux)") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	//create stub output (for unused demuxer's outputs)
	auto null = uptr(Out::Null::create());

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
			videoIndex = i;
		} else {
			ConnectPinToModule(demux->getPin(i), null); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	auto props = demux->getPin(videoIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);

	auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
	auto encode = uptr(Encode::LibavEncode::create(Encode::LibavEncode::Video));
	auto mux = uptr(new Mux::GPACMuxMP4("output_video_gpac"));

	//pass meta data between encoder and mux
	Connect(encode->declareStream, mux.get(), &Mux::GPACMuxMP4::declareStream);
	encode->sendOutputPinsInfo();

	ConnectPinToModule(demux->getPin(videoIndex), decode);
	ConnectPinToModule(decode->getPin(0), encode);
	ConnectPinToModule(encode->getPin(0), mux);

	demux->process(nullptr);
}

#if 0
unittest("transcoder: audio simple (libav mux)") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	//create stub output (for unused demuxer's outputs)
	auto null = uptr(Out::Null::create());

	//find video signal from demux
	size_t audioIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		Props *props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_AUDIO) { //TODO: expose it somewhere
			audioIndex = i;
		} else {
			ConnectPinToModule(demux->getPin(i), null); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(audioIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	Props *props = demux->getPin(audioIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
	auto decode = uptr(Decode::LibavDecode::create(*decoderProps));

	//create the encoder
	auto encode = uptr(Encode::LibavEncode::create(Encode::LibavEncode::Audio));
	auto mux = uptr(Mux::LibavMux::create("output_audio_libav"));

	//pass meta data between encoder and mux
	Connect(encode->declareStream, mux.get(), &Mux::LibavMux::declareStream);
	encode->sendOutputPinsInfo();

	//create an audio resampler
	auto audioConverter = uptr(Transform::AudioConvert::create());

	ConnectPinToModule(demux->getPin(audioIndex), decode);
	ConnectPinToModule(decode->getPin(0), audioConverter);
	ConnectPinToModule(audioConverter->getPin(0), encode);
	ConnectPinToModule(encode->getPin(0), mux);

	demux->process(nullptr);
}
#endif

#if 0
unittest("transcoder: audio simple (gpac mux)") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	//create stub output (for unused demuxer's outputs)
	auto null = uptr(Out::Null::create());

	//find video signal from demux
	size_t audioIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		Props *props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_AUDIO) { //TODO: expose it somewhere
			audioIndex = i;
		} else {
			ConnectToModule(demux->getPin(i)->getSignal(), null); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(audioIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	Props *props = demux->getPin(audioIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);

	auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
	auto encode = uptr(Encode::LibavEncode::create(Encode::LibavEncode::Audio));
	auto mux = uptr(Mux::LibavMux::create("output_audio_gpac"));

	//pass meta data between encoder and mux
	Connect(encode->declareStream, mux.get(), &Mux::LibavMux::declareStream);
	encode->sendOutputPinsInfo();

	//create an audio resampler
	auto audioConverter = uptr(Transform::AudioConvert::create());

	ConnectToModule(demux->getPin(audioIndex)->getSignal(), decode);
	ConnectToModule(decode->getPin(0)->getSignal(), audioConverter);
	ConnectToModule(audioConverter->getPin(0)->getSignal(), encode);
	ConnectToModule(encode->getPin(0)->getSignal(), mux);

	demux->process(nullptr);
}
#endif

}
