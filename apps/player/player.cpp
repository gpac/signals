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
std::unique_ptr<Module> renderPin(Pin* pPin, int codec_type) {
	if (codec_type == AVMEDIA_TYPE_VIDEO) {
		Log::msg(Log::Info, "Found video stream");
		auto r = uptr(new Render::SDLVideo);
		ConnectPin(pPin, r.get(), &Render::SDLVideo::process);
		return std::move(r);
	} else if (codec_type == AVMEDIA_TYPE_AUDIO) {
		Log::msg(Log::Info, "Found audio stream");
		auto r = uptr(Render::SDLAudio::create());
		ConnectPin(pPin, r.get(), &Render::SDLAudio::process);
		return std::move(r);
	} else {
		Log::msg(Log::Info, "Found unknown stream");
		auto r = uptr(Out::Null::create());
		ConnectPin(pPin, r.get(), &Out::Null::process);
		return std::move(r);
	}
}
}

int safeMain(int argc, char const* argv[]) {

	if(argc != 2)
		throw std::runtime_error("usage: player <file>");

	auto const inputFile = argv[1];

	std::list<std::unique_ptr<Module>> modules;

	{
		auto demux = uptr(Demux::LibavDemux::create(inputFile));

		for (size_t i = 0; i < demux->getNumPin(); ++i) {
			auto props = demux->getPin(i)->getProps();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);

			auto decoder = uptr(Decode::LibavDecode::create(*decoderProps));
			ConnectPin(demux->getPin(i), decoder.get(), &Decode::LibavDecode::process);

			auto const codec_type = decoderProps->getAVCodecContext()->codec_type;
			auto renderer = renderPin(decoder->getPin(0), codec_type);

			modules.push_back(std::move(decoder));
			modules.push_back(std::move(renderer));
		}

		while (demux->process(nullptr)) {
		}

		foreach(i, modules) {
			(*i)->waitForCompletion();
		}
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


