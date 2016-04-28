#include "lib_modules/modules.hpp"
#include "lib_media/media.hpp"
#include "pipeliner.hpp"
#include <sstream>

using namespace Modules;
using namespace Pipelines;

namespace {
ModuleS* createRenderer(int codecType) {
	if (codecType == VIDEO_PKT) {
		Log::msg(Info, "Found video stream");
		return create<Render::SDLVideo>();
	} else if (codecType == AUDIO_PKT) {
		Log::msg(Info, "Found audio stream");
		return create<Render::SDLAudio>();
	} else {
		Log::msg(Info, "Found unknown stream");
		return create<Out::Null>();
	}
}
}

void declarePipeline(Pipeline &pipeline, const char *url) {
	auto connect = [&](auto* src, auto* dst) {
		pipeline.connect(src, 0, dst, 0);
	};

	auto demux = pipeline.addModule(create<Demux::LibavDemux>(url));
	for (int i = 0; i < (int)demux->getNumOutputs(); ++i) {
		auto metadata = getMetadataFromOutput<MetadataPktLibav>(demux->getOutput(i));
		auto decode = pipeline.addModule(create<Decode::LibavDecode>(*metadata));

		pipeline.connect(demux, i, decode, 0);

		auto render = pipeline.addModule(createRenderer(metadata->getStreamType()));
		if (!render)
			continue;

		connect(decode, render);
	}
}
