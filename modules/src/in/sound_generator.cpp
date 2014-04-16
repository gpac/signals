#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "sound_generator.hpp"
#include <cmath>

#ifndef M_PI //FIXME: Cygwin does not have maths.h extensions
#define M_PI 3.14159265358979323846
#endif

auto const SAMPLE_RATE = 44100;
auto const SINE_FREQ = 880.0;

namespace Modules {
namespace In {

SoundGenerator::SoundGenerator()
	: m_numSamples(0) {
	signals.push_back(uptr(pinFactory->createPin()));
}

bool SoundGenerator::process(std::shared_ptr<Data> /*data*/) {
	auto const bytesPerSample = 4;
	auto const sampleDurationInMs = 40;
	auto const bufferSize = bytesPerSample * sampleDurationInMs * SAMPLE_RATE / 1000;
	std::shared_ptr<PcmData> out(new PcmData(bufferSize));

	out->setTime(m_numSamples * 180000LL / SAMPLE_RATE);

	// generate sound
	auto const p = out->data();
	for(int i=0; i < (int)out->size()/bytesPerSample; ++i) {
		auto const fVal = nextSample();
		auto const val = int(fVal * 32767.0f);

		// left
		p[i*bytesPerSample+0] = (val >> 0) & 0xFF;
		p[i*bytesPerSample+1] = (val >> 8) & 0xFF;

		// right
		p[i*bytesPerSample+2] = (val >> 0) & 0xFF;
		p[i*bytesPerSample+3] = (val >> 8) & 0xFF;
	}

	signals[0]->emit(out);
	return true;
}

double SoundGenerator::nextSample() {
	auto const BEEP_PERIOD = SAMPLE_RATE;
	auto const beepPhase = m_numSamples % BEEP_PERIOD;
	auto const phase = m_numSamples * 2.0 * SINE_FREQ * M_PI / SAMPLE_RATE;
	auto const fVal = beepPhase < BEEP_PERIOD/8 ? sin(phase) : 0;
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
