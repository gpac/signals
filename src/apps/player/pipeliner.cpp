#include "lib_modules/modules.hpp"
#include "lib_media/media.hpp"
#include "pipeliner.hpp"
#include <sstream>

namespace {
Module* createRenderer(int codecType) {
	if (codecType == VIDEO_PKT) {
		Log::msg(Log::Info, "Found video stream");
		return new Render::SDLVideo();
	} else if (codecType == AUDIO_PKT) {
		Log::msg(Log::Info, "Found audio stream");
		return new Render::SDLAudio();
	} else {
		Log::msg(Log::Info, "Found unknown stream");
		return new Out::Null;
	}
}
}

void declarePipeline(Pipeline &pipeline, const char *url) {
	auto connect = [&](PipelinedModule* src, PipelinedModule* dst) {
		pipeline.connect(src->getOutputPin(0), dst);
	};

	auto demux = pipeline.addModule(new Demux::LibavDemux(url), true);
	for (int i = 0; i < (int)demux->getNumOutputPins(); ++i) {
		auto props = demux->getOutputPin(i)->getProps();
		auto decoderProps = safe_cast<PropsDecoder>(props);
		auto decoder = pipeline.addModule(new Decode::LibavDecode(*decoderProps));

		pipeline.connect(demux->getOutputPin(i), decoder);

		auto const codecType = decoderProps ? decoderProps->getStreamType() : UNKNOWN_ST;
		auto renderer = pipeline.addModule(createRenderer(codecType));
		if (!renderer)
			continue;

		connect(decoder, renderer);
	}
}
