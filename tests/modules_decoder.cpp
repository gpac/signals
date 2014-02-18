#include "tests.hpp"
#include "modules.hpp"
#include <memory>
#include "../utils/tools.hpp"

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

using namespace Tests;
using namespace Modules;

unittest("decoder: audio simple") {

	auto input = uptr(In::File::create("data/sine.mp3"));

	//create the audio decoder
	AVCodecContext avContext;
	memset(&avContext, 0, sizeof avContext);
	avContext.codec_type = AVMEDIA_TYPE_AUDIO;
	avContext.codec_id = AV_CODEC_ID_MP3;
	PropsDecoder props(&avContext);
	auto decoder = uptr(Decode::LibavDecode::create(props));
	Connect(input->getPin(0)->getSignal(), decoder.get(), &Decode::LibavDecode::process);

	auto null = uptr(Out::Null::create());
	Connect(decoder->getPin(0)->getSignal(), null.get(), &Out::Null::process);

	//create an audio resampler
	// std::unique_ptr<Transform::AudioConvert> audioConverter(Transform::AudioConvert::create());
	//Connect(audioConverter->signals[0]->signal, encode.get(), &Encode::LibavEncode::process);

	while (input->process(nullptr)) {
	}

	input->waitForCompletion();
	decoder->waitForCompletion();
}

