#include "../../tests/tests.hpp"
#include "lib_utils/tools.hpp"
#include "lib_modules/modules.hpp"

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

#include "lib_media/decode/libav_decode.hpp"
#include "lib_media/demux/libav_demux.hpp"
#include "lib_media/out/null.hpp"
#include "lib_media/render/sdl_audio.hpp"
#include "lib_media/render/sdl_video.hpp"

using namespace Tests;
using namespace Modules;

//-----------------------------------------------------------------------------

struct Stream {
	Stream(Module* from) : fromModule(from) {
		pin = nullptr;
	}
	std::shared_ptr<Module> fromModule;
	IPin* pin;

	Stream(Stream const& s) = delete;
	Stream(Stream&& s) = default;
};

Module* createRenderer(int codecType, IClock* clock) {
	if (codecType == AVMEDIA_TYPE_VIDEO) {
		Log::msg(Log::Info, "Found video stream");
		return new Render::SDLVideo(clock);
	} else if (codecType == AVMEDIA_TYPE_AUDIO) {
		Log::msg(Log::Info, "Found audio stream");
		return Render::SDLAudio::create(clock);
	} else {
		Log::msg(Log::Info, "Found unknown stream");
		return new Out::Null;
	}
}

//-----------------------------------------------------------------------------

std::vector<Stream> demux(std::string filename) {
	std::vector<Stream> r;
	auto demux = Demux::LibavDemux::create(filename);

	for (size_t i = 0; i < demux->getNumPin(); ++i) {
		Stream s(demux);
		s.pin = demux->getPin(i);
		r.push_back(std::move(s));
	}

	return r;
}

Stream decode(Stream& input) {
	auto props = input.pin->getProps();
	auto decoderProps = safe_cast<PropsDecoder>(props);

	auto decoder = new Decode::LibavDecode(*decoderProps);
	Stream r(decoder);
	r.pin = decoder->getPin(0);
	ConnectPinToModule(input.pin, decoder, defaultExecutor);

	return r;
}

int getCodecType(Stream& input)
{
	auto props = input.pin->getProps();
	auto decoderProps = safe_cast<PropsDecoder>(props);
	return decoderProps->getAVCodecContext()->codec_type;
}

Stream render(Stream& input, IClock* clock) {
	auto const codecType = getCodecType(input);

	Stream r(createRenderer(codecType, clock));
	ConnectPinToModule(input.pin, r.fromModule.get(), defaultExecutor);
	return r;
}

int safeMain(int argc, char const* argv[]) {

	if(argc != 2)
		throw std::runtime_error("usage: player <file>");

	auto const inputFile = argv[1];

	auto clock = uptr(createSystemClock());

	// construct pipeline
	auto es = demux(inputFile);
	auto decodedStreams = apply(&decode, es);
	auto renderedStreams = apply(&render, decodedStreams, clock.get());

	// pump all data
	auto srcModule = es[0].fromModule;
	srcModule->process(nullptr);

	return 0;
}

int main(int argc, char const* argv[]) {
	try {
		return safeMain(argc, argv);
	} catch(std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 0;
	}
}


