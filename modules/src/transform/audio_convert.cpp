#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "audio_convert.hpp"

namespace Modules {
namespace Transform {

AudioConvert::AudioConvert() {
	signals.push_back(uptr(pinFactory->createPin()));
}

AudioConvert::~AudioConvert() {
}

AudioConvert* AudioConvert::create() {
	return new AudioConvert();
}

void AudioConvert::process(std::shared_ptr<Data> data) {
	assert(0);
	//signals[0]->emit(out);
}

}
}
