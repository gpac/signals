#include "../../tests/tests.hpp"
#include "modules.hpp"

#include "../utils/tools.hpp"
#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

#include "decode/libav_decode.hpp"
#include "demux/libav_demux.hpp"
#include "out/null.hpp"
#include "render/sdl_audio.hpp"
#include "render/sdl_video.hpp"

#include "pipeline.hpp"

using namespace Tests;
using namespace Modules;

//-----------------------------------------------------------------------------

struct Stream {
	Stream(Module* from) : fromModule(from) {
		pin = nullptr;
	}
	std::shared_ptr<Module> fromModule;
	Pin* pin;

	Stream(Stream const& s) = delete;
	Stream(Stream&& s) = default;

	//FIXME: TO BE REMOVED
	int codec_type; 
};

Module* createRenderer(int codec_type) {
	if (codec_type == AVMEDIA_TYPE_VIDEO) {
		Log::msg(Log::Info, "Found video stream");
		return new Render::SDLVideo;
	} else if (codec_type == AVMEDIA_TYPE_AUDIO) {
		Log::msg(Log::Info, "Found audio stream");
		return Render::SDLAudio::create();
	} else {
		Log::msg(Log::Info, "Found unknown stream");
		return Out::Null::create();
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
	auto decoderProps = dynamic_cast<PropsDecoder*>(props);
	ASSERT(decoderProps);

	auto decoder = Decode::LibavDecode::create(*decoderProps);
	Stream r(decoder);
	r.pin = decoder->getPin(0);
	r.codec_type = decoderProps->getAVCodecContext()->codec_type;
	ConnectPin(input.pin, decoder->getInput());

	return r;
}

Stream render(Stream& input) {
	Stream r(createRenderer(input.codec_type));
	ConnectPin(input.pin, r.fromModule->getInput());
	return r;
}

Stream waitForCompletion(Stream& stream) {
	stream.fromModule->waitForCompletion();
	return Stream(nullptr);
}

int safeMain(int argc, char const* argv[]) {

	if(argc != 2)
		throw std::runtime_error("usage: player <file>");

	auto const inputFile = argv[1];

	// construct pipeline
	auto es = demux(inputFile);
	auto decodedStreams = apply(&decode, es);
	auto renderedStreams = apply(&render, decodedStreams);

	// pump all data
	auto srcModule = es[0].fromModule;
	while (srcModule->process(nullptr)) {
	}

	apply(&waitForCompletion, renderedStreams);
	apply(&waitForCompletion, decodedStreams);
	apply(&waitForCompletion, es);

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


