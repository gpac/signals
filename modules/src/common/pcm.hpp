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
static const int AUDIO_CHANNEL_NUM = 2;
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

	bool isComparable(AudioPcmConfig *cfg) const {
		if (cfg &&
			(cfg->sampleRate == sampleRate) &&
			(cfg->numChannels == numChannels) &&
			(cfg->layout == layout) &&
			(cfg->format == format) &&
			(cfg->numPlanes == numPlanes)) {
			return true;
		}

		return false;
	}

	uint8_t getBytesPerSample() const {
		uint8_t b = 1;
		switch (format) {
		case S16: b *= 2; break;
		case F32: b *= 4; break;
		default: throw std::runtime_error("Unknown audio format");
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

	uint8_t getLayout() const {
		return layout;
	}

	uint8_t getNumPlanes() const {
		return numPlanes;
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
		if (numPlanes > AUDIO_PCM_PLANES_MAX)
			throw std::runtime_error("Too many Pcm planes.");
		this->numPlanes = numPlanes;
	}

private:
	uint32_t sampleRate;
	uint8_t numChannels;
	AudioLayout layout;

	AudioFormat format;
	uint8_t numPlanes;
};

//Romain: rename PcmData in DataPcm
class PcmData : public Data, public AudioPcmConfig {
public:
	PcmData(size_t size) : Data(0) {
		memset(planes, 0, sizeof(planes));
		memset(planeSize, 0, sizeof(planeSize));
		if (size > 0) {
			setNumPlanes(1);
			setPlane(0, nullptr, size);
		}
	}

	~PcmData() {
		freePlanes();
	}

	virtual uint8_t* data() {
		if (getNumPlanes() > 1)
			throw std::runtime_error("Forbidden operation. Use audio planes to access the data.");
		return planes[0];
	}

	virtual uint64_t size() const {
		if (getNumPlanes() > 1)
			throw std::runtime_error("Forbidden operation. Use audio planes to retrieve the size.");
		return planeSize[0];
	}

	virtual void resize(size_t size) {
		throw std::runtime_error("Forbidden operation. You cannot resize PCM data.");
	}

	uint8_t* getPlane(size_t planeIdx) const {
		if (planeIdx > getNumPlanes())
			throw std::runtime_error("Pcm plane doesn't exist.");
		return planes[planeIdx];
	}

	uint64_t getPlaneSize(size_t planeIdx) const {
		if (planeIdx > getNumPlanes())
			throw std::runtime_error("Pcm plane doesn't exist.");
		return planeSize[planeIdx];
	}

	void setPlanes(uint8_t numAudioPlanes, uint8_t* audioPlanes[AUDIO_PCM_PLANES_MAX], uint64_t audioPlaneSize[AUDIO_PCM_PLANES_MAX]) {
		setNumPlanes(numAudioPlanes);
		freePlanes();
		for (uint8_t i = 0; i < numAudioPlanes; ++i) {
			setPlane(i, planes[i], planeSize[i]);
		}
	}

	void setPlane(uint8_t planeIdx, uint8_t *plane, uint64_t size) {
		if (planeIdx > getNumPlanes())
			throw std::runtime_error("Pcm plane doesn't exist.");
		freePlane(planeIdx);
		planes[planeIdx] = new uint8_t[size];
		planeSize[planeIdx] = size;
		if (plane) {
			memcpy(planes[planeIdx], plane, size);
		}
	}

private:
	void freePlane(uint8_t planeIdx) {
		delete planes[planeIdx];
		planes[planeIdx] = nullptr;
		planeSize[planeIdx] = 0;
	}
	void freePlanes() {
		for (uint8_t i = 0; i < getNumPlanes(); ++i) {
			freePlane(i);
		}
	}

	uint8_t* planes[AUDIO_PCM_PLANES_MAX];
	uint64_t planeSize[AUDIO_PCM_PLANES_MAX];
};

typedef PinDataDefault<PcmData> PinPcm;

class PinPcmFactory : public PinFactory {
public:
	PinPcmFactory() {
	}
	Pin* createPin(IProps *props = nullptr) {
		return new PinPcm(props);
	}
};

}
