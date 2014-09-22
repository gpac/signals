#include "tests.hpp"
#include "modules.hpp"
#include "../utils/tools.hpp"

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

#include "decode/libav_decode.hpp"
#include "in/file.hpp"
#include "out/null.hpp"
#include "transform/audio_convert.hpp"

using namespace Tests;
using namespace Modules;

namespace {
Decode::LibavDecode* createMp3Decoder() {
	AVCodecContext avContext;
	memset(&avContext, 0, sizeof avContext);
	avContext.codec_type = AVMEDIA_TYPE_AUDIO;
	avContext.codec_id = AV_CODEC_ID_MP3;
	PropsDecoder props(&avContext);
	return Decode::LibavDecode::create(props);
}
}

unittest("decoder: audio simple") {

	auto input = uptr(In::File::create("data/sine.mp3"));

	//create the audio decoder
	auto decoder = uptr(createMp3Decoder());
	ConnectPinToModule(input->getPin(0), decoder.get());

	auto null = uptr(Out::Null::create());
	ConnectPinToModule(decoder->getPin(0), null.get());

	//create an audio resampler
	//auto audioConverter = uptr(Transform::AudioConvert::create());
	//Connect(audioConverter->getPin(0)->getSignal(), decoder.get(), &Encode::LibavEncode::process);

	input->process(nullptr);

	input->waitForCompletion();
	decoder->waitForCompletion();
}

