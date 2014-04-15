#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "sound_generator.hpp"
#include <cmath>

auto const BUFFER_SIZE = 128;
auto const SAMPLE_RATE = 44100;
auto const SINE_FREQ = 440.0;

namespace Modules {
namespace In {

SoundGenerator::SoundGenerator()
	: m_numSamples(0) {
	signals.push_back(uptr(pinFactory->createPin()));
}

bool SoundGenerator::process(std::shared_ptr<Data> /*data*/) {
	std::shared_ptr<PcmData> out(new PcmData(BUFFER_SIZE));

	// generate sound
	auto const bytesPerSample = 2;
  auto const p = out->data();
	for(int i=0;i < BUFFER_SIZE/bytesPerSample;++i) {
		auto const fVal = nextSample();
		auto const val = int(fVal * 32767.0f);
		p[i*2+0] = (val >> 0) & 0xFF;
		p[i*2+1] = (val >> 8) & 0xFF;
	}

	signals[0]->emit(out);
	return true;
}

double SoundGenerator::nextSample() {
	auto const phase = m_numSamples * 2.0 * SINE_FREQ * M_PI / SAMPLE_RATE;
	auto const fVal = sin(phase);
	m_numSamples++;
	return fVal;
}

bool SoundGenerator::handles(const std::string &url) {
	return SoundGenerator::canHandle(url);
}

bool SoundGenerator::canHandle(const std::string &/*url*/) {
	return true;
}

}
}
