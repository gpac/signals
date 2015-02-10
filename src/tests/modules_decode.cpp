#include "tests.hpp"
#include "modules.hpp"

extern "C"
{
#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level
}

#include "decode/libav_decode.hpp"
#include "encode/libav_encode.hpp"
#include "in/file.hpp"
#include "out/null.hpp"
#include "transform/audio_convert.hpp"
#include "tools.hpp"

using namespace Tests;
using namespace Modules;

namespace {
Decode::LibavDecode* createGenericDecoder(enum AVCodecID id) {
	auto codec = avcodec_find_decoder(id);
	auto context = avcodec_alloc_context3(codec);
	PropsDecoder props(context);
	auto decode = new Decode::LibavDecode(props);
	avcodec_close(context);
	return decode;
}

Decode::LibavDecode* createMp3Decoder() {
	return createGenericDecoder(AV_CODEC_ID_MP3);
}

template<size_t numBytes>
std::shared_ptr<Data> createAvPacket(uint8_t const (&bytes)[numBytes]) {
	auto pkt = std::make_shared<DataAVPacket>(numBytes);
	memcpy(pkt->data(), bytes, numBytes);
	return pkt;
}

std::shared_ptr<Data> getTestMp3Frame() {
	static const uint8_t mp3_sine_frame[] = {
		0xff, 0xfb, 0x30, 0xc0, 0x00, 0x00, 0x00, 0x00,
	 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x49, 0x6e, 0x66, 0x6f,
	 	0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x29,
		0x00, 0x00, 0x19, 0xb6, 0x00, 0x0c, 0x0c, 0x12,
	 	0x12, 0x18, 0x18, 0x18, 0x1e, 0x1e, 0x24, 0x24,
		0x24, 0x2a, 0x2a, 0x30, 0x30, 0x30, 0x36, 0x36,
	 	0x3c, 0x3c, 0x43, 0x43, 0x43, 0x49, 0x49, 0x4f,
		0x4f, 0x4f, 0x55, 0x55, 0x5b, 0x5b, 0x5b, 0x61,
	 	0x61, 0x67, 0x67, 0x67, 0x6d, 0x6d, 0x73, 0x73,
		0x79, 0x79, 0x79, 0x7f, 0x7f, 0x86, 0x86, 0x86,
	 	0x8c, 0x8c, 0x92, 0x92, 0x92, 0x98, 0x98, 0x9e,
		0x9e, 0xa4, 0xa4, 0xa4, 0xaa, 0xaa, 0xb0, 0xb0,
	 	0xb0, 0xb6, 0xb6, 0xbc, 0xbc, 0xbc, 0xc3, 0xc3,
		0xc9, 0xc9, 0xc9, 0xcf, 0xcf, 0xd5, 0xd5, 0xdb,
	 	0xdb, 0xdb, 0xe1, 0xe1, 0xe7, 0xe7, 0xe7, 0xed,
		0xed, 0xf3, 0xf3, 0xf3, 0xf9, 0xf9, 0xff, 0xff,
	 	0x00, 0x00, 0x00, 0x00
	};

	return createAvPacket(mp3_sine_frame);
}

}

unittest("decoder: audio simple") {
	auto decoder = uptr(createMp3Decoder());

	auto null = uptr(new Out::Null);
	ConnectPinToModule(decoder->getPin(0), null);

	auto frame = getTestMp3Frame();
	decoder->process(frame);
}

namespace {
Decode::LibavDecode* createVideoDecoder() {
	return createGenericDecoder(AV_CODEC_ID_H264);
}

std::shared_ptr<Data> getTestH24Frame() {
  static const uint8_t h264_gray_frame[] = {
    0x00, 0x00, 0x00, 0x01,
    0x67, 0x4d, 0x40, 0x0a, 0xe8, 0x8f, 0x42, 0x00,
    0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x03, 0x00,
    0x64, 0x1e, 0x24, 0x4a, 0x24,
    0x00, 0x00, 0x00, 0x01, 0x68, 0xeb, 0xc3, 0xcb,
    0x20, 0x00, 0x00, 0x01, 0x65, 0x88, 0x84, 0x00,
    0xaf, 0xfd, 0x0f, 0xdf,
  };

	return createAvPacket(h264_gray_frame);
}
}

unittest("decoder: video simple") {
	auto decoder = uptr(createVideoDecoder());
	auto data = getTestH24Frame();

	auto onPic = [&](std::shared_ptr<const Data> data) {
		auto const pic = safe_cast<const Picture>(data);
		auto const res = pic->getResolution();
		ASSERT_EQUALS(16, res.width);
		ASSERT_EQUALS(16, res.height);

		auto const firstPixel = *pic->getComp(0);
		auto const lastPixel = *(pic->getComp(0) + res.width * res.height - 1);
		ASSERT_EQUALS(0x80, firstPixel);
		ASSERT_EQUALS(0x80, lastPixel);
	};

	ConnectPin(decoder->getPin(0), onPic);
	decoder->process(data);
	decoder->process(data);
}

#ifdef ENABLE_FAILING_TESTS
//TODO: this test fails because the exception is caught by a Signals future. To be tested when tasks are pushed to an executor
unittest("decoder: failing audio mp3 to AAC") {
	auto decoder = uptr(createMp3Decoder());
	auto encoder = uptr(new Encode::LibavEncode(Encode::LibavEncode::Audio));

	ConnectPinToModule(decoder->getPin(0), encoder);

	auto frame = getTestMp3Frame();
	bool thrown = false;
	try {
		decoder->process(frame);
	} catch (std::exception const& e) {
		std::cerr << "Expected error: " << e.what() << std::endl;
		thrown = true;
	}
	ASSERT(thrown);
}
#endif

unittest("decoder: audio mp3 to converter to AAC") {
	auto decoder = uptr(createMp3Decoder());
	auto encoder = uptr(new Encode::LibavEncode(Encode::LibavEncode::Audio));

	auto srcFormat = PcmFormat(44100, 1, AudioLayout::Mono, AudioSampleFormat::S16, AudioStruct::Planar);
	auto dstFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
	auto converter = uptr(new Transform::AudioConvert(srcFormat, dstFormat));

	ConnectPinToModule(decoder->getPin(0), converter);
	ConnectPinToModule(converter->getPin(0), encoder);

	auto frame = getTestMp3Frame();
	decoder->process(frame);
}


