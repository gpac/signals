#include "../utils/log.hpp"
#include "audio_convert.hpp"

namespace Modules {
namespace Transform {

AudioConvert::AudioConvert() {
	signals.push_back(new Pin());
}

AudioConvert::~AudioConvert() {
	delete signals[0];
}

AudioConvert* AudioConvert::create() {
	return new AudioConvert();
}

bool AudioConvert::process(std::shared_ptr<Data> data) {
	assert(0);
	//signals[0]->emit(out);
	return false;
}

bool AudioConvert::handles(const std::string &url) {
	return AudioConvert::canHandle(url);
}

bool AudioConvert::canHandle(const std::string &/*url*/) {
	return true;
}

}
}
