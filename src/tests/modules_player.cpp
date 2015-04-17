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
	auto demux = uptr(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(new Out::Null);

	size_t videoIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumOutputPins(); ++i) {
		auto metadata = getMetadataFromPin<MetadataPktLibav>(demux->getOutputPin(i));
		if (metadata->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoIndex = i;
		} else {
			ConnectPinToModule(demux->getOutputPin(i), null);
		}
	}
	ASSERT(videoIndex != std::numeric_limits<size_t>::max());
	auto metadata = getMetadataFromPin<MetadataPktLibav>(demux->getOutputPin(videoIndex));
	auto decode = uptr(new Decode::LibavDecode(*metadata));
	auto render = uptr(new Render::SDLVideo);

	ConnectPinToModule(demux->getOutputPin(videoIndex), decode);
	ConnectPinToModule(decode->getOutputPin(0), render);

	demux->process(nullptr);
}

unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Audio Only) -> Render::SDL2") {
	auto demux = uptr(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
	auto null = uptr(new Out::Null);

	size_t audioIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < demux->getNumOutputPins(); ++i) {
		auto metadata = getMetadataFromPin<IMetadataPkt>(demux->getOutputPin(i));
		if (metadata->getStreamType() == AUDIO_PKT) {
			audioIndex = i;
		} else {
			ConnectPinToModule(demux->getOutputPin(i), null);
		}
	}
	ASSERT(audioIndex != std::numeric_limits<size_t>::max());
	auto metadata = getMetadataFromPin<MetadataPktLibav>(demux->getOutputPin(audioIndex));
	auto decode = uptr(new Decode::LibavDecode(*metadata));
	auto srcFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::F32, AudioStruct::Planar);
	auto dstFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
	auto converter = uptr(new Transform::AudioConvert(srcFormat, dstFormat));
	auto render = uptr(new Render::SDLAudio());

	ConnectPinToModule(demux->getOutputPin(audioIndex), decode);
	ConnectPinToModule(decode->getOutputPin(0), converter);
	ConnectPinToModule(converter->getOutputPin(0), render);

	demux->process(nullptr);
}

}
