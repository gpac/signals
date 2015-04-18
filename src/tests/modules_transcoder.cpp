#include "tests.hpp"
#include "lib_modules/modules.hpp"
#include "lib_media/decode/jpegturbo_decode.hpp"
#include "lib_media/decode/libav_decode.hpp"
#include "lib_media/demux/libav_demux.hpp"
#include "lib_media/encode/jpegturbo_encode.hpp"
#include "lib_media/encode/libav_encode.hpp"
#include "lib_media/in/file.hpp"
#include "lib_media/mux/libav_mux.hpp"
#include "lib_media/mux/gpac_mux_mp4.hpp"
#include "lib_media/out/file.hpp"
#include "lib_media/out/null.hpp"
#include "lib_media/transform/video_convert.hpp"
#include "lib_utils/tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("transcoder: video simple (libav mux)") {
	auto demux = uptr(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(new Out::Null);

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumOutputs(); ++i) {
		auto metadata = getMetadataFromOutput<IMetadataPkt>(demux->getOutput(i));
		if (metadata->getStreamType() == VIDEO_PKT) {
			videoIndex = i;
		} else {
			ConnectOutputToModule(demux->getOutput(i), null);
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	//create the video decode
	auto metadata = getMetadataFromOutput<MetadataPktLibav>(demux->getOutput(videoIndex));
	auto decode = uptr(new Decode::LibavDecode(*metadata));
	auto encode = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
	auto mux = uptr(new Mux::LibavMux("output_video_libav"));

	ConnectOutputToModule(demux->getOutput(videoIndex), decode);
	ConnectOutputToModule(decode->getOutput(0), encode);
	ConnectOutputToModule(encode->getOutput(0), mux);

	demux->process(nullptr);
}

unittest("transcoder: video simple (gpac mux)") {
	auto demux = uptr(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	//create stub output (for unused demuxer's outputs)
	auto null = uptr(new Out::Null);

	//find video signal from demux
	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumOutputs(); ++i) {
		auto metadata = getMetadataFromOutput<IMetadataPkt>(demux->getOutput(i));
		if (metadata->getStreamType() == VIDEO_PKT) {
			videoIndex = i;
		} else {
			ConnectOutputToModule(demux->getOutput(i), null);
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());

	//create the video decode
	auto metadata = getMetadataFromOutput<MetadataPktLibav>(demux->getOutput(videoIndex));
	auto decode = uptr(new Decode::LibavDecode(*metadata));
	auto encode = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
	auto mux = uptr(new Mux::GPACMuxMP4("output_video_gpac"));

	ConnectOutputToModule(demux->getOutput(videoIndex), decode);
	ConnectOutputToModule(decode->getOutput(0), encode);
	ConnectOutputToModule(encode->getOutput(0), mux);

	demux->process(nullptr);
}

#ifdef ENABLE_FAILING_TESTS
//TODO: we have no media type when connecting
unittest("transcoder: jpg to jpg") {
	const std::string filename("data/sample.jpg");
	auto decode = uptr(new Decode::JPEGTurboDecode());
	{
		auto preReader = uptr(new In::File(filename));
		ConnectOutputToModule(preReader->getOutput(0), decode);
		preReader->process(nullptr);
	}

	auto reader = uptr(new In::File(filename));
	auto metadata = getMetadataFromOutput<MetadataPktLibavVideo>(decode->getOutput(0));
	auto dstRes = metadata->getResolution();
	auto encoder = uptr(new Encode::JPEGTurboEncode(dstRes));
	auto writer = uptr(new Out::File("data/test.jpg"));

	ConnectOutputToModule(reader->getOutput(0), decode);
	ConnectOutputToModule(decode->getOutput(0), encoder);
	ConnectOutputToModule(encoder->getOutput(0), writer);

	reader->process(nullptr);
}

unittest("transcoder: jpg to resized jpg") {
	const std::string filename("data/sample.jpg");
	auto decode = uptr(new Decode::JPEGTurboDecode());
	{
		auto preReader = uptr(new In::File(filename));
		ConnectOutputToModule(preReader->getOutput(0), decode);
		preReader->process(nullptr);
	}
	auto reader = uptr(new In::File(filename));

	auto metadata = getMetadataFromOutput<MetadataPktLibavVideo>(decode->getOutput(0));
	ASSERT(metadata->getPixelFormat() == RGB24);
	auto dstRes = metadata->getResolution() / 2;
	auto dstFormat = PictureFormat(dstRes, metadata->getPixelFormat());
	auto converter = uptr(new Transform::VideoConvert(dstFormat));
	auto encoder = uptr(new Encode::JPEGTurboEncode(dstRes));
	auto writer = uptr(new Out::File("data/test.jpg"));

	ConnectOutputToModule(reader->getOutput(0), decode);
	ConnectOutputToModule(decode->getOutput(0), converter);
	ConnectOutputToModule(converter->getOutput(0), encoder);
	ConnectOutputToModule(encoder->getOutput(0), writer);

	reader->process(nullptr);
}
#endif

unittest("transcoder: h264/mp4 to jpg") {
	auto demux = uptr(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));

	auto metadata = getMetadataFromOutput<MetadataPktLibavVideo>(demux->getOutput(0));
	auto decode = uptr(new Decode::LibavDecode(*metadata));

	auto dstRes = metadata->getResolution();
	auto encoder = uptr(new Encode::JPEGTurboEncode(dstRes));
	auto writer = uptr(new Out::File("data/test.jpg"));

	ASSERT(metadata->getPixelFormat() == YUV420P);
	auto dstFormat = PictureFormat(dstRes, RGB24);
	auto converter = uptr(new Transform::VideoConvert(dstFormat));

	ConnectOutputToModule(demux->getOutput(0), decode);
	ConnectOutputToModule(decode->getOutput(0), converter);
	ConnectOutputToModule(converter->getOutput(0), encoder);
	ConnectOutputToModule(encoder->getOutput(0), writer);

	demux->process(nullptr);
}

#ifdef ENABLE_FAILING_TESTS
//TODO: we have no media type when connecting
unittest("transcoder: jpg to h264/mp4 (gpac)") {
	const std::string filename("data/sample.jpg");
	auto decode = uptr(new Decode::JPEGTurboDecode());
	{
		auto preReader = uptr(new In::File(filename));
		ConnectOutputToModule(preReader->getOutput(0), decode);
		//FIXME: to retrieve the metadata, we now need to decode (need to have a memory module keeping the data while inspecting)
		preReader->process(nullptr);
	}
	auto reader = uptr(new In::File(filename));

	auto metadata = getMetadataFromOutput<MetadataPktLibavVideo>(decode->getOutput(0));
	auto srcRes = metadata->getResolution();
	ASSERT(metadata->getPixelFormat() == RGB24);
	auto dstFormat = PictureFormat(srcRes, metadata->getPixelFormat());
	auto converter = uptr(new Transform::VideoConvert(dstFormat));

	auto encoder = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
	auto mux = uptr(new Mux::GPACMuxMP4("data/test"));

	ConnectOutputToModule(reader->getOutput(0), decode);
	ConnectOutputToModule(decode->getOutput(0), converter);
	ConnectOutputToModule(converter->getOutput(0), encoder);
	ConnectOutputToModule(encoder->getOutput(0), mux);

	reader->process(nullptr);
}
#endif

}
