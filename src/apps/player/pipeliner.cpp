#include "lib_modules/modules.hpp"
#include "lib_media/media.hpp"
#include "pipeliner.hpp"
#include <sstream>

namespace {
ModuleS* createRenderer(int codecType) {
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
	auto connect = [&](auto* src, auto* dst) {
		pipeline.connect(src, 0, dst, 0);
	};

	auto demux = pipeline.addModule(new Demux::LibavDemux(url), true);
	for (int i = 0; i < (int)demux->getNumOutputs(); ++i) {
		auto metadata = getMetadataFromOutput<MetadataPktLibav>(demux->getOutput(i));
		auto decode = pipeline.addModule(new Decode::LibavDecode(*metadata));

		pipeline.connect(demux, i, decode, 0);

		auto render = pipeline.addModule(createRenderer(metadata->getStreamType()));
		if (!render)
			continue;

		connect(decode, render);
	}
}
