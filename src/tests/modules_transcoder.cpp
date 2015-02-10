#include "tests.hpp"
#include "modules.hpp"

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

#include "decode/jpegturbo_decode.hpp"
#include "decode/libav_decode.hpp"
#include "demux/libav_demux.hpp"
#include "encode/jpegturbo_encode.hpp"
#include "encode/libav_encode.hpp"
#include "in/file.hpp"
#include "mux/libav_mux.hpp"
#include "mux/gpac_mux_mp4.hpp"
#include "out/file.hpp"
#include "out/null.hpp"
#include "transform/video_convert.hpp"
#include "tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("transcoder: video simple (libav mux)") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(new Out::Null);

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoIndex = i;
		} else {
			ConnectPinToModule(demux->getPin(i), null);
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	auto props = demux->getPin(videoIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);

	auto decode = uptr(new Decode::LibavDecode(*decoderProps));
	auto encode = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
	auto mux = uptr(Mux::LibavMux::create("output_video_libav"));

	ConnectPinToModule(demux->getPin(videoIndex), decode);
	ConnectPinToModule(decode->getPin(0), encode);
	ConnectPinToModule(encode->getPin(0), mux);
	encode->sendOutputPinsInfo();

	demux->process(nullptr);
}

unittest("transcoder: video simple (gpac mux)") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	//create stub output (for unused demuxer's outputs)
	auto null = uptr(new Out::Null);

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoIndex = i;
		} else {
			ConnectPinToModule(demux->getPin(i), null);
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	//create the video decoder
	auto props = demux->getPin(videoIndex)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);

	auto decode = uptr(new Decode::LibavDecode(*decoderProps));
	auto encode = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
	auto mux = uptr(new Mux::GPACMuxMP4("output_video_gpac"));

	ConnectPinToModule(demux->getPin(videoIndex), decode);
	ConnectPinToModule(decode->getPin(0), encode);
	ConnectPinToModule(encode->getPin(0), mux);
	encode->sendOutputPinsInfo();

	demux->process(nullptr);
}

unittest("transcoder: jpg to jpg") {
	const std::string filename("data/sample.jpg");
	auto decoder = uptr(new Decode::JPEGTurboDecode());
	{
		auto preReader = uptr(In::File::create(filename));
		ConnectPinToModule(preReader->getPin(0), decoder);
		preReader->process(nullptr);
	}
	auto props = decoder->getPin(0)->getProps();
	ASSERT(props != nullptr);
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
	auto srcCtx = decoderProps->getAVCodecContext();

	auto reader = uptr(In::File::create(filename));
	auto dstRes = Resolution(decoderProps->getAVCodecContext()->width, srcCtx->height);
	auto encoder = uptr(new Encode::JPEGTurboEncode(dstRes));
	auto writer = uptr(Out::File::create("data/test.jpg"));

	ConnectPinToModule(reader->getPin(0), decoder);
	ConnectPinToModule(decoder->getPin(0), encoder);
	ConnectPinToModule(encoder->getPin(0), writer);

	reader->process(nullptr);
}

unittest("transcoder: jpg to resized jpg") {
	const std::string filename("data/sample.jpg");
	auto decoder = uptr(new Decode::JPEGTurboDecode());
	{
		auto preReader = uptr(In::File::create(filename));
		ConnectPinToModule(preReader->getPin(0), decoder);
		preReader->process(nullptr);
	}
	auto props = decoder->getPin(0)->getProps();
	ASSERT(props != nullptr);
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
	auto srcCtx = decoderProps->getAVCodecContext();

	auto srcRes = Resolution(srcCtx->width, srcCtx->height);
	auto dstRes = Resolution(srcCtx->width / 2, srcCtx->height / 2);

	auto reader = uptr(In::File::create(filename));
	auto converter = uptr(new Transform::VideoConvert(srcRes, srcCtx->pix_fmt, dstRes, srcCtx->pix_fmt));
	auto encoder = uptr(new Encode::JPEGTurboEncode(dstRes));
	auto writer = uptr(Out::File::create("data/test.jpg"));

	ConnectPinToModule(reader->getPin(0), decoder);
	ConnectPinToModule(decoder->getPin(0), converter);
	ConnectPinToModule(converter->getPin(0), encoder);
	ConnectPinToModule(encoder->getPin(0), writer);

	reader->process(nullptr);
}

unittest("transcoder: h264/mp4 to jpg") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	auto props = demux->getPin(0)->getProps();
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
	auto decoder = uptr(new Decode::LibavDecode(*decoderProps));

	auto srcCtx = decoderProps->getAVCodecContext();
	auto srcRes = Resolution(srcCtx->width, srcCtx->height);
	auto encoder = uptr(new Encode::JPEGTurboEncode(srcRes));
	auto writer = uptr(Out::File::create("data/test.jpg"));

	auto converter = uptr(new Transform::VideoConvert(
				srcRes, srcCtx->pix_fmt,
			 	srcRes, AV_PIX_FMT_RGB24));

	ConnectPinToModule(demux->getPin(0), decoder);
	ConnectPinToModule(decoder->getPin(0), converter);
	ConnectPinToModule(converter->getPin(0), encoder);
	ConnectPinToModule(encoder->getPin(0), writer);

	demux->process(nullptr);
}

unittest("transcoder: jpg to h264/mp4 (gpac)") {
	const std::string filename("data/sample.jpg");
	auto decoder = uptr(new Decode::JPEGTurboDecode());
	{
		auto preReader = uptr(In::File::create(filename));
		ConnectPinToModule(preReader->getPin(0), decoder);
		//FIXME: to retrieve the props, we now need to decode (need to have a memory module keeping the data while inspecting)
		preReader->process(nullptr);
	}
	auto props = decoder->getPin(0)->getProps();
	ASSERT(props != nullptr);
	PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
	auto srcCtx = decoderProps->getAVCodecContext();
	auto srcRes = Resolution(srcCtx->width, srcCtx->height);

	auto reader = uptr(In::File::create(filename));
	auto converter = uptr(new Transform::VideoConvert(
				srcRes, srcCtx->pix_fmt,
			 	srcRes, AV_PIX_FMT_YUV420P));

	auto encoder = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
	auto mux = uptr(new Mux::GPACMuxMP4("data/test"));

	ConnectPinToModule(reader->getPin(0), decoder);
	ConnectPinToModule(decoder->getPin(0), converter);
	ConnectPinToModule(converter->getPin(0), encoder);
	ConnectPinToModule(encoder->getPin(0), mux);
	encoder->sendOutputPinsInfo();

	reader->process(nullptr);
}

}
