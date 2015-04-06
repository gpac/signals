extern "C" {
#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level
}
#include "tests.hpp"
#include "lib_modules/modules.hpp"
#include "lib_utils/tools.hpp"
#include "lib_media/decode/libav_decode.hpp"
#include "lib_media/demux/libav_demux.hpp"
#include "lib_media/out/null.hpp"
#include "lib_media/render/sdl_audio.hpp"
#include "lib_media/render/sdl_video.hpp"
#include "lib_media/transform/audio_convert.hpp"
#include "lib_utils/tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {

unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Video Only) -> Render::SDL2") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(new Out::Null);

	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		auto decoderProps = safe_cast<PropsDecoder>(props);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose types it somewhere else
			videoIndex = i;
		} else {
			ConnectPinToModule(demux->getPin(i), null);
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());
	auto props = demux->getPin(videoIndex)->getProps();
	auto decoderProps = safe_cast<PropsDecoder>(props);
	auto decode = uptr(new Decode::LibavDecode(*decoderProps));
	auto render = uptr(new Render::SDLVideo);

	ConnectPinToModule(demux->getPin(videoIndex), decode);
	ConnectPinToModule(decode->getPin(0), render);

	demux->process(nullptr);
}

unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Audio Only) -> Render::SDL2") {
	auto demux = uptr(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(new Out::Null);

	size_t audioIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		auto decoderProps = safe_cast<PropsDecoder>(props);
		if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioIndex = i;
		} else {
			ConnectPinToModule(demux->getPin(i), null);
		}
	}
	ASSERT(audioIndex != std::numeric_limits<size_t>::max());
	auto props = demux->getPin(audioIndex)->getProps();
	auto decoderProps = safe_cast<PropsDecoder>(props);
	auto decode = uptr(new Decode::LibavDecode(*decoderProps));
	auto srcFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::F32, AudioStruct::Planar);
	auto dstFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
	auto converter = uptr(new Transform::AudioConvert(srcFormat, dstFormat));
	auto render = uptr(new Render::SDLAudio());

	ConnectPinToModule(demux->getPin(audioIndex), decode);
	ConnectPinToModule(decode->getPin(0), converter);
	ConnectPinToModule(converter->getPin(0), render);

	demux->process(nullptr);
}

}
