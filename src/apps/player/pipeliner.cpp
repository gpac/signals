extern "C" {
#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level
}
#include "lib_media/media.hpp"
#include "pipeliner.hpp"
#include <sstream>

namespace {
Module* createRenderer(int codecType) {
	if (codecType == AVMEDIA_TYPE_VIDEO) {
		Log::msg(Log::Info, "Found video stream");
		return new Render::SDLVideo();
	} else if (codecType == AVMEDIA_TYPE_AUDIO) {
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
		pipeline.connect(src->getPin(0), dst);
	};

	auto demux = pipeline.addModule(Demux::LibavDemux::create(url), true);
	for (int i = 0; i < (int)demux->getNumPin(); ++i) {
		auto props = demux->getPin(i)->getProps();
		auto decoderProps = safe_cast<PropsDecoder>(props);
		auto decoder = pipeline.addModule(new Decode::LibavDecode(*decoderProps));

		pipeline.connect(demux->getPin(i), decoder);

		auto const codecType = decoderProps ? decoderProps->getAVCodecContext()->codec_type : AVMEDIA_TYPE_UNKNOWN;
		auto renderer = pipeline.addModule(createRenderer(codecType));
		if (!renderer)
			continue;

		connect(decoder, renderer);
	}
}
