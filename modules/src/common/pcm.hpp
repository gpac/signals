#pragma once

#include "internal/core/data.hpp"
#include "internal/core/pin.hpp"


namespace Modules {

enum AudioFormat {
	S16,
	F32
};

enum AudioLayout {
	Mono,
	Stereo
};

static const int AUDIO_SAMPLERATE = 44100;
static const int AUDIO_CHANNEL_NUM = 1;
static const AudioLayout AUDIO_LAYOUT = Stereo;

static const AudioFormat AUDIO_PCM_FORMAT = S16;
static const int AUDIO_PCM_PLANES_DEFAULT = 1;
static const int AUDIO_PCM_PLANES_MAX = 8;


class AudioPcmConfig {
public:
	AudioPcmConfig(uint32_t sampleRate = AUDIO_SAMPLERATE, uint8_t numChannels = AUDIO_CHANNEL_NUM,
		AudioLayout layout = AUDIO_LAYOUT, AudioFormat format = AUDIO_PCM_FORMAT, uint8_t numPlanes = AUDIO_PCM_PLANES_DEFAULT) :
		sampleRate(sampleRate), numChannels(numChannels), layout(layout), format(format), numPlanes(numPlanes) {
	}

	void setConfig(AudioPcmConfig &audioPcmConfig) {
		setConfig(audioPcmConfig.sampleRate, audioPcmConfig.numChannels, audioPcmConfig.layout, audioPcmConfig.format, audioPcmConfig.numPlanes);
	}

	void setConfig(uint32_t sampleRate, uint8_t numChannels, AudioLayout layout, AudioFormat format, uint8_t numPlanes) {
		setSampleRate(sampleRate);
		setChannels(numChannels, layout);
		setFormat(format);
		setNumPlanes(numPlanes);
	}

	bool isComparable(AudioPcmConfig &cfg) {
		if ((cfg.sampleRate == sampleRate) &&
			(cfg.numChannels == numChannels) &&
			(cfg.layout == layout) &&
			(cfg.format == format) &&
			(cfg.numPlanes == numPlanes)) {
			return true;
		}

		return false;
	}

	uint8_t getBytesPerSample() {
		uint8_t b = 1;
		switch (format) {
		case S16:
			b *= 2;
			break;
		case F32:
			b *= 4;
			break;
		default:
			throw std::runtime_error("Unknown audio format");
		}
		switch (layout) {
		case Mono:
			b *= 1;
			break;
		case Stereo:
			b *= 2;
			break;
		default:
			throw std::runtime_error("Unknown audio layout");
		}

		return b;
	}

	uint32_t getSampleRate() const {
		return sampleRate;
	}

	AudioFormat getFormat() const {
		return format;
	}

	uint8_t getNumChannels() const {
		return numChannels;
	}

	void setSampleRate(uint32_t sampleRate) {
		this->sampleRate = sampleRate;
	}

	void setChannels(uint8_t numChannels, AudioLayout layout) {
		this->numChannels = numChannels;
		this->layout = layout;
	}

	void setFormat(AudioFormat format) {
		this->format = format;
	}

	void setNumPlanes(uint8_t numPlanes) {
		this->numPlanes = numPlanes;
	}

private:
	uint32_t sampleRate;
	uint8_t numChannels;
	AudioLayout layout;

	AudioFormat format;
	uint8_t numPlanes;

#if 0 //Romain
	memset(planes, 0, sizeof(planes));
	void setNumPlanes(uint8_t numPlanes, uint8_t* planes[AUDIO_PCM_PLANES_MAX]) {
		this->numPlanes = numPlanes;
		memcpy(this->planes, planes, sizeof(this->planes));
	}
	uint8_t* planes[AUDIO_PCM_PLANES_MAX];
#endif
};

class PcmData : public Data, public AudioPcmConfig {
public:
	PcmData(size_t size) : Data(size), AudioPcmConfig() {
	}

	virtual uint8_t* data() {
		throw std::runtime_error("Forbiden operation. Use audio planes to access the data.");
	}

	virtual uint64_t size() const {
		throw std::runtime_error("Forbiden operation. Use audio planes to retrieve the size.");
	}

	virtual void resize(size_t size) {
		throw std::runtime_error("Forbiden operation. You cannot resize PCM data.");
	}
};

typedef PinDataDefault<PcmData> PinPcm;

//Romain: template
class PinPcmFactory : public PinFactory {
public:
	PinPcmFactory() {
	}
	Pin* createPin(IProps *props = nullptr) {
		return new PinPcm(props);
	}
};

}
