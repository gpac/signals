#include "../../tests/tests.hpp"
#include "modules.hpp"

#include "../utils/tools.hpp"
#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

#include "decode/libav_decode.hpp"
#include "demux/libav_demux.hpp"
#include "out/null.hpp"
#include "render/sdl_audio.hpp"
#include "render/sdl_video.hpp"

using namespace Tests;
using namespace Modules;

namespace {
Module* renderPin(Pin* pPin, int codec_type) {
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

class Pipeline {
public:
	void add(Module* module) {
		modules.push_back(uptr(module));
	}

	void run() {
		auto& sourceModule = modules[0];
		while (sourceModule->process(nullptr)) {
		}
	}

	~Pipeline() {
		foreach(i, modules) {
			(*i)->waitForCompletion();
		}
	}

private:
	std::vector<std::unique_ptr<Module>> modules;
};
}

int safeMain(int argc, char const* argv[]) {

	if(argc != 2)
		throw std::runtime_error("usage: player <file>");

	auto const inputFile = argv[1];

	{
		Pipeline pipeline;

		auto demux = Demux::LibavDemux::create(inputFile);
		pipeline.add(demux);

		for (size_t i = 0; i < demux->getNumPin(); ++i) {
			auto props = demux->getPin(i)->getProps();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);

			auto decoder = Decode::LibavDecode::create(*decoderProps);
			pipeline.add(decoder);

			ConnectPin(demux->getPin(i), decoder->getInput());

			auto const codec_type = decoderProps->getAVCodecContext()->codec_type;
			auto renderer = renderPin(decoder->getPin(0), codec_type);
			pipeline.add(renderer);

			ConnectPin(decoder->getPin(0), renderer->getInput());
		}

		pipeline.run();
	}

	return 0;
}

int main(int argc, char const* argv[]) {
	try {
		return safeMain(argc, argv);
	} catch(std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}


