#include "lib_modules/modules.hpp"
#include "lib_media/media.hpp"
#include "pipeliner.hpp"
#include <sstream>

using namespace Modules;
using namespace Pipelines;

//#define DEBUG_MONITOR

namespace {
Encode::LibavEncode* createEncoder(std::shared_ptr<const IMetadata> metadata, const dashcastXOptions &opt, size_t i) {
	auto const codecType = metadata->getStreamType();
	if (codecType == VIDEO_PKT) {
		Log::msg(Log::Info, "[Encoder] Found video stream");
		Encode::LibavEncodeParams p;
		p.isLowLatency = opt.isLive;
		p.res = opt.v[i].res;
		p.bitrate_v = opt.v[i].bitrate;
		return new Encode::LibavEncode(Encode::LibavEncode::Video, p);
	} else if (codecType == AUDIO_PKT) {
		Log::msg(Log::Info, "[Encoder] Found audio stream");
		return new Encode::LibavEncode(Encode::LibavEncode::Audio);
	} else {
		Log::msg(Log::Info, "[Encoder] Found unknown stream");
		return nullptr;
	}
}

ModuleS* createConverter(std::shared_ptr<const IMetadata> metadata, const Resolution &dstRes) {
	auto const codecType = metadata->getStreamType();
	if (codecType == VIDEO_PKT) {
		Log::msg(Log::Info, "[Converter] Found video stream");
		auto dstFormat = PictureFormat(dstRes, YUV420P);
		return new Transform::VideoConvert(dstFormat);
	} else if (codecType == AUDIO_PKT) {
		Log::msg(Log::Info, "[Converter] Found audio stream");
		auto format = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
		return new Transform::AudioConvert(format);
	} else {
		Log::msg(Log::Info, "[Converter] Found unknown stream");
		return nullptr;
	}
}
}

void declarePipeline(Pipeline &pipeline, const dashcastXOptions &opt) {
	auto connect = [&](auto* src, auto* dst) {
		pipeline.connect(src, 0, dst, 0);
	};

	auto demux = pipeline.addModule(new Demux::LibavDemux(opt.url));
	auto dasher = pipeline.addModule(new Modules::Stream::MPEG_DASH("dashcastx.mpd",
	                                 opt.isLive ? Modules::Stream::MPEG_DASH::Live : Modules::Stream::MPEG_DASH::Static, opt.segmentDuration));

	int numDashInputs = 0;
	for (size_t i = 0; i < demux->getNumOutputs(); ++i) {
		auto const metadata = getMetadataFromOutput<MetadataPktLibav>(demux->getOutput(i));
		if (!metadata) {
			Log::msg(Log::Warning, "[DashcastX] Unknown metadata for stream %s. Ignoring.", i);
			break;
		}

		auto decode = pipeline.addModule(new Decode::LibavDecode(*metadata));
		pipeline.connect(demux, i, decode, 0);

#ifdef DEBUG_MONITOR
		auto webcamPreview = pipeline.addModule(new Render::SDLVideo());
		connect(decode, webcamPreview);
#endif

		auto const numRes = metadata->isVideo() ? opt.v.size() : 1;
		for (size_t r = 0; r < numRes; ++r) {
			//TODO: add optional bitrate a/o transcode
			auto converter = pipeline.addModule(createConverter(metadata, opt.v[r].res));
			if (!converter)
				continue;

			connect(decode, converter);

			auto rawEncoder = createEncoder(metadata, opt, r);
			auto encoder = pipeline.addModule(rawEncoder);
			if (!encoder)
				continue;

			connect(converter, encoder);

			std::stringstream filename;
			filename << numDashInputs;
			auto muxer = pipeline.addModule(new Mux::GPACMuxMP4(filename.str(), opt.segmentDuration, true));
			connect(encoder, muxer);

			pipeline.connect(muxer, 0, dasher, numDashInputs);
			numDashInputs++;
		}
	}
}
