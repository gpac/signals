#include "../../tests/tests.hpp"
#include "modules.hpp"

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

#include "decode/libav_decode.hpp"
#include "demux/libav_demux.hpp"
#include "encode/libav_encode.hpp"
#include "mux/gpac_mux_mp4.hpp"
#include "stream/mpeg_dash.hpp"
#include "transform/audio_convert.hpp"
#include "out/null.hpp"

#include "../../utils/tools.hpp"

#include <sstream>

using namespace Tests;
using namespace Modules;

namespace {
std::unique_ptr<Encode::LibavEncode> createEncoder(Pin *pPin, PropsDecoder *decoderProps) {
	auto const codecType = decoderProps ? decoderProps->getAVCodecContext()->codec_type : AVMEDIA_TYPE_UNKNOWN;
	if (codecType == AVMEDIA_TYPE_VIDEO) {
		Log::msg(Log::Info, "Found video stream");
		auto r = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
		ConnectPinToModule(pPin, r);
		return std::move(r);
	} else if (codecType == AVMEDIA_TYPE_AUDIO) {
		Log::msg(Log::Info, "Found audio stream");
		auto r = uptr(new Encode::LibavEncode(Encode::LibavEncode::Audio));
		ConnectPinToModule(pPin, r);
		return std::move(r);
	} else {
		Log::msg(Log::Info, "Found unknown stream");
		return nullptr;
	}
}
}

int safeMain(int argc, char const* argv[]) {

	if(argc != 2)
		throw std::runtime_error("usage: dashcastx <URL>");

	auto const inputURL = argv[1];

	std::list<std::unique_ptr<Module>> modules;

	{
		auto demux = uptr(Demux::LibavDemux::create(inputURL));
		auto dasher = uptr(new Modules::Stream::MPEG_DASH(Modules::Stream::MPEG_DASH::Static));

		for (size_t i = 0; i < demux->getNumPin(); ++i) {
			auto props = demux->getPin(i)->getProps();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);

			auto decoder = uptr(new Decode::LibavDecode(*decoderProps));
			ConnectPinToModule(demux->getPin(i), decoder);

			//FIXME: hardcoded converters
			Pin *pPin;
			if (i == 0) {
				pPin = decoder->getPin(0);
			} else {
				auto baseFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::F32, AudioStruct::Planar);
				auto otherFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
				auto converter = uptr(new Transform::AudioConvert(baseFormat, otherFormat));
				ConnectPinToModule(decoder->getPin(0), converter);
				pPin = converter->getPin(0);
				modules.push_back(std::move(converter));
			}

			auto encoder = createEncoder(pPin, decoderProps);
			if (!encoder) {
				auto r = uptr(new Out::Null);
				ConnectPinToModule(decoder->getPin(0), r);
				modules.push_back(std::move(decoder));
				modules.push_back(std::move(r));
				continue;
			}

			std::stringstream filename;
			filename << i;
			auto muxer = uptr(new Mux::GPACMuxMP4(filename.str(), true));
			ConnectPinToModule(encoder->getPin(0), muxer);

			Connect(encoder->declareStream, muxer.get(), &Mux::GPACMuxMP4::declareStream);
			encoder->sendOutputPinsInfo();

			//FIXME: hardcoded => use declareStream above
			if (i == 0) {
				Connect(muxer->getPin(0)->getSignal(), dasher.get(), &Modules::Stream::MPEG_DASH::processVideo);
			} else {
				Connect(muxer->getPin(0)->getSignal(), dasher.get(), &Modules::Stream::MPEG_DASH::processAudio);
			}

			modules.push_back(std::move(decoder));
			modules.push_back(std::move(encoder));
			modules.push_back(std::move(muxer));
		}

		modules.push_back(std::move(dasher));
		demux->process(nullptr);
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
