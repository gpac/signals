#include "lib_modules/modules.hpp"
#include "lib_media/media.hpp"
#include "pipeliner.hpp"
#include <sstream>

namespace {
Encode::LibavEncode* createEncoder(std::shared_ptr<const IMetadata> metadata, const dashcastXOptions &opt) {
	auto const codecType = metadata->getStreamType();
	if (codecType == VIDEO_PKT) {
		Log::msg(Log::Info, "[Encoder] Found video stream");
		Encode::LibavEncodeParams p;
		p.isLowLatency = opt.isLive;
		p.res = opt.res;
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
		auto imageMetadata = safe_cast<const MetadataPktLibavVideo>(metadata);
		auto dstFormat = PictureFormat(dstRes, imageMetadata->getPixelFormat());
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
	auto dasher = pipeline.addModule(new Modules::Stream::MPEG_DASH(
		opt.isLive ? Modules::Stream::MPEG_DASH::Live : Modules::Stream::MPEG_DASH::Static, opt.segmentDuration));

	for (int i = 0; i < (int)demux->getNumOutputs(); ++i) {
		auto metadata = getMetadataFromOutput<MetadataPktLibav>(demux->getOutput(i));
		auto decode = pipeline.addModule(new Decode::LibavDecode(*metadata));

		pipeline.connect(demux, i, decode, 0);

		auto converter = pipeline.addModule(createConverter(metadata, opt.res));
		if (!converter)
			continue;

		connect(decode, converter);

		auto rawEncoder = createEncoder(metadata, opt);
		auto encoder = pipeline.addModule(rawEncoder);
		if (!encoder)
			continue;

		connect(converter, encoder);

		std::stringstream filename;
		filename << i;
		auto muxer = pipeline.addModule(new Mux::GPACMuxMP4(filename.str(), true, opt.segmentDuration));
		connect(encoder, muxer);

		connect(muxer, dasher);
	}
}
