#include "lib_modules/modules.hpp"
#include "lib_media/media.hpp"
#include "pipeliner.hpp"
#include <sstream>

namespace {
Encode::LibavEncode* createEncoder(IMetadataPkt *metadata, bool isLowLatency) {
	auto const codecType = metadata->getStreamType();
	if (codecType == VIDEO_PKT) {
		Log::msg(Log::Info, "[Encoder] Found video stream");
		return new Encode::LibavEncode(Encode::LibavEncode::Video, isLowLatency);
	} else if (codecType == AUDIO_PKT) {
		Log::msg(Log::Info, "[Encoder] Found audio stream");
		return new Encode::LibavEncode(Encode::LibavEncode::Audio);
	} else {
		Log::msg(Log::Info, "[Encoder] Found unknown stream");
		return nullptr;
	}
}

Module* createConverter(IMetadataPkt *metadata, const Resolution &dstRes) {
	auto const codecType = metadata->getStreamType();
	if (codecType == VIDEO_PKT) {
		Log::msg(Log::Info, "[Converter] Found video stream");
		auto imageMetadata = safe_cast<MetadataPktLibavVideo>(metadata);
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
	auto connect = [&](PipelinedModule* src, PipelinedModule* dst) {
		pipeline.connect(src->getOutputPin(0), dst);
	};

	auto demux = pipeline.addModule(new Demux::LibavDemux(opt.url), true);
	auto dasher = pipeline.addModule(new Modules::Stream::MPEG_DASH(
		opt.isLive ? Modules::Stream::MPEG_DASH::Live : Modules::Stream::MPEG_DASH::Static, opt.segmentDuration));

	for (int i = 0; i < (int)demux->getNumOutputPins(); ++i) {
		auto metadata = safe_cast<Metadata>(demux->getOutputPin(i))->getMetadata();
		auto decoder = pipeline.addModule(new Decode::LibavDecode(*safe_cast<MetadataPktLibav>(metadata)));

		pipeline.connect(demux->getOutputPin(i), decoder);

		auto converter = pipeline.addModule(createConverter(metadata, opt.res));
		if (!converter)
			continue;

		connect(decoder, converter);

		auto rawEncoder = createEncoder(metadata, opt.isLive);
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
