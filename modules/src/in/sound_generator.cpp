#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "internal/core/clock.hpp"
#include "sound_generator.hpp"
#include <cmath>

#ifndef M_PI //FIXME: Cygwin does not have maths.h extensions
#define M_PI 3.14159265358979323846
#endif

namespace Modules {
namespace In {

auto const SAMPLE_RATE = AUDIO_SAMPLERATE;
auto const SINE_FREQ = 880.0;

SoundGenerator::SoundGenerator()
	: m_numSamples(20000) {
	audioCfg.setSampleRate(SAMPLE_RATE);
	PinPcmFactory pinFactory;
	pins.push_back(uptr(pinFactory.createPin()));
}

void SoundGenerator::process(std::shared_ptr<Data> /*data*/) {
	auto const bytesPerSample = audioCfg.getBytesPerSample();
	auto const sampleDurationInMs = 40;
	auto const bufferSize = bytesPerSample * sampleDurationInMs * audioCfg.getSampleRate() / 1000;
	
	auto out = safe_cast<PcmData>(pins[0]->getBuffer(bufferSize));
	out->setConfig(audioCfg);
	out->setTime(m_numSamples * IClock::Rate / audioCfg.getSampleRate());

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

	pins[0]->emit(out);
}

double SoundGenerator::nextSample() {
	auto const BEEP_PERIOD = audioCfg.getSampleRate();
	auto const beepPhase = m_numSamples % BEEP_PERIOD;
	auto const phase = m_numSamples * 2.0 * SINE_FREQ * M_PI / audioCfg.getSampleRate();
	auto const fVal = beepPhase < BEEP_PERIOD/8 ? sin(phase) : 0;
	m_numSamples++;
	return fVal;
}

}
}
