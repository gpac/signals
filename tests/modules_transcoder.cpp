#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

using namespace Tests;
using namespace Modules;

namespace {

unittest("transcoder: video simple (libav mux)") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(Out::Null::create());

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		Props *props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
			videoIndex = i;
		} else {
			ConnectPin(demux->getPin(i), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	Props *props = demux->getPin(videoIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);

	auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
	auto encode = uptr(Encode::LibavEncode::create(Encode::LibavEncode::Video));
	auto mux = uptr(Mux::LibavMux::create("output_video_libav"));

	//pass meta data between encoder an mux
	Connect(encode->declareStream, mux.get(), &Mux::LibavMux::declareStream);
	encode->sendOutputPinsInfo();

	ConnectPin(demux->getPin(videoIndex), decode.get(), &Decode::LibavDecode::process);
	ConnectPin(decode->getPin(0), encode.get(), &Encode::LibavEncode::process);
	ConnectPin(encode->getPin(0), mux.get(), &Mux::LibavMux::process);

	while (demux->process(nullptr)) {
	}

	demux->waitForCompletion();
	decode->waitForCompletion();
	mux->waitForCompletion();
	encode->waitForCompletion();
}

unittest("transcoder: video simple (gpac mux)") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	//create stub output (for unused demuxer's outputs)
	auto null = uptr(Out::Null::create());

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		Props *props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
			videoIndex = i;
		} else {
			ConnectPin(demux->getPin(i), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	Props *props = demux->getPin(videoIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);

	auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
	auto encode = uptr(Encode::LibavEncode::create(Encode::LibavEncode::Video));
	auto mux = uptr(Mux::GPACMuxMP4::create("output_video_gpac"));

	//pass meta data between encoder an mux
	Connect(encode->declareStream, mux.get(), &Mux::GPACMuxMP4::declareStream);
	encode->sendOutputPinsInfo();

	ConnectPin(demux->getPin(videoIndex), decode.get(), &Decode::LibavDecode::process);
	ConnectPin(decode->getPin(0), encode.get(), &Encode::LibavEncode::process);
	ConnectPin(encode->getPin(0), mux.get(), &Mux::GPACMuxMP4::process);

	while (demux->process(nullptr)) {
	}

	demux->waitForCompletion();
	decode->waitForCompletion();
	mux->waitForCompletion();
	encode->waitForCompletion();
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
			ConnectPin(demux->getPin(i), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
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

	//pass meta data between encoder an mux
	Connect(encode->declareStream, mux.get(), &Mux::LibavMux::declareStream);
	encode->sendOutputPinsInfo();

	//create an audio resampler
	auto audioConverter = uptr(Transform::AudioConvert::create());

	ConnectPin(demux->getPin(audioIndex), decode.get(), &Decode::LibavDecode::process);
	ConnectPin(decode->getPin(0), audioConverter.get(), &Transform::AudioConvert::process);
	ConnectPin(audioConverter->getPin(0), encode.get(), &Encode::LibavEncode::process);
	ConnectPin(encode->getPin(0), mux.get(), &Mux::LibavMux::process);

	while (demux->process(nullptr)) {
	}

	demux->waitForCompletion();
	decode->waitForCompletion();
	mux->waitForCompletion();
	encode->waitForCompletion();
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
			Connect(demux->getPin(i)->getSignal(), null.get(), &Out::Null::process); //FIXME: this is a stub to void the assert of not connected signals...
		}
	}
	ASSERT(audioIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	Props *props = demux->getPin(audioIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);

	auto decode = uptr(Decode::LibavDecode::create(*decoderProps));
	auto encode = uptr(Encode::LibavEncode::create(Encode::LibavEncode::Audio));
	auto mux = uptr(Mux::LibavMux::create("output_audio_gpac"));

	//pass meta data between encoder an mux
	Connect(encode->declareStream, mux.get(), &Mux::LibavMux::declareStream);
	encode->sendOutputPinsInfo();

	//create an audio resampler
	auto audioConverter = uptr(Transform::AudioConvert::create());

	Connect(demux->getPin(audioIndex)->getSignal(), decode.get(), &Decode::LibavDecode::process);
	Connect(decode->getPin(0)->getSignal(), audioConverter.get(), &Transform::AudioConvert::process);
	Connect(audioConverter->getPin(0)->getSignal(), encode.get(), &Encode::LibavEncode::process);
	Connect(encode->getPin(0)->getSignal(), mux.get(), &Mux::LibavMux::process);

	while (demux->process(nullptr)) {
	}

	demux->waitForCompletion();
	decode->waitForCompletion();
	mux->waitForCompletion();
	encode->waitForCompletion();
}
#endif

}
