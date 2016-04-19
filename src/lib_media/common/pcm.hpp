#pragma once

#include "lib_modules/core/data.hpp"
#include "lib_modules/core/output.hpp"


namespace Modules {

enum AudioSampleFormat {
	S16,
	F32
};

enum AudioLayout {
	Mono,
	Stereo
};

enum AudioStruct {
	Interleaved,
	Planar
};

static const uint32_t AUDIO_SAMPLERATE = 44100;
static const uint8_t AUDIO_CHANNEL_NUM = 2;
static const AudioLayout AUDIO_LAYOUT = Stereo;

static const AudioSampleFormat AUDIO_PCM_FORMAT = F32;
static const uint8_t AUDIO_PCM_PLANES_MAX = 8;
}

namespace {
uint8_t getNumChannelsFromLayout(Modules::AudioLayout layout) {
	switch (layout) {
	case Modules::Mono: return 1;
	case Modules::Stereo: return 2;
	default: throw std::runtime_error("Unknown audio layout");
	}
}
}

namespace Modules {

class PcmFormat {
	public:
		PcmFormat(uint32_t sampleRate = AUDIO_SAMPLERATE, uint8_t numChannels = AUDIO_CHANNEL_NUM,
		          AudioLayout layout = AUDIO_LAYOUT, AudioSampleFormat sampleFormat = AUDIO_PCM_FORMAT, AudioStruct structa = Planar) :
			sampleRate(sampleRate), numChannels(numChannels), layout(layout), sampleFormat(sampleFormat), numPlanes((structa == Planar) ? numChannels : 1) {
		}

		PcmFormat(uint32_t sampleRate, AudioLayout layout, AudioSampleFormat sampleFormat, AudioStruct structa) :
			sampleRate(sampleRate), numChannels(getNumChannelsFromLayout(layout)), layout(layout), sampleFormat(sampleFormat), numPlanes((structa == Planar) ? numChannels : 1) {
		}

		bool operator!=(const PcmFormat& other) const {
			return !(*this == other);
		}

		PcmFormat& operator=(const PcmFormat& other) {
			if (this != &other) {
				sampleRate = other.sampleRate;
				numChannels = other.numChannels;
				layout = other.layout;
				sampleFormat = other.sampleFormat;
				numPlanes = other.numPlanes;
			}
			return *this;
		}

		bool operator==(const PcmFormat& other) const {
			if (other.sampleRate != sampleRate) {
				Log::msg(Info, "[Audio] Incompatible configuration: sample rate is %s, expect %s.", other.sampleRate, sampleRate);
				return false;
			}
			if (other.numChannels != numChannels) {
				Log::msg(Info, "[Audio] Incompatible configuration: channel number is %s, expect %s.", other.numChannels, numChannels);
				return false;
			}
			if (other.layout != layout) {
				Log::msg(Info, "[Audio] Incompatible configuration: layout is %s, expect %s.", other.layout, layout);
				return false;
			}
			if (other.sampleFormat != sampleFormat) {
				Log::msg(Info, "[Audio] Incompatible configuration: sample format is %s, expect %s.", other.sampleFormat, sampleFormat);
				return false;
			}
			if (other.numPlanes != numPlanes) {
				Log::msg(Info, "[Audio] Incompatible configuration: plane number is %s, expect %s.", other.numPlanes, numPlanes);
				return false;
			}

			return true;
		}

		uint8_t getBytesPerSample() const {
			uint8_t b = 1;
			switch (sampleFormat) {
			case S16: b *= 2; break;
			case F32: b *= 4; break;
			default: throw std::runtime_error("Unknown audio format");
			}
			b *= getNumChannelsFromLayout(layout);
			return b;
		}

		uint32_t sampleRate;
		uint8_t numChannels;
		AudioLayout layout;

		AudioSampleFormat sampleFormat;
		uint8_t numPlanes;
};

class DataPcm : public DataRaw {
	public:
		DataPcm(size_t size) : DataRaw(0) {
			memset(planes, 0, sizeof(planes));
			memset(planeSize, 0, sizeof(planeSize));
			if (size > 0) {
				throw std::runtime_error("Forbidden operation. Requested size must be 0. Then call setFormat().");
				format.numPlanes = 1;
				setPlane(0, nullptr, size);
			}
		}
		~DataPcm() {
			freePlanes();
		}
		virtual bool isRecyclable() const override {
			return false;
		}

		const PcmFormat& getFormat() const {
			return format;
		}

		void setFormat(PcmFormat const& format) {
			this->format = format;
		}

		uint8_t* data() override {
			if (format.numPlanes > 1)
				throw std::runtime_error("Forbidden operation. Use audio planes to access the data.");
			return planes[0];
		}

		uint8_t const* data() const override {
			if (format.numPlanes > 1)
				throw std::runtime_error("Forbidden operation. Use audio planes to access the data.");
			return planes[0];
		}

		uint64_t size() const override {
			uint64_t size = 0;
			for (size_t i = 0; i < format.numPlanes; ++i) {
				size += planeSize[i];
			}
			return size;
		}

		virtual void resize(size_t size) override {
			throw std::runtime_error("Forbidden operation. You cannot resize PCM data.");
		}

		uint8_t* getPlane(size_t planeIdx) const {
			if (planeIdx > format.numPlanes)
				throw std::runtime_error("Pcm plane doesn't exist.");
			return planes[planeIdx];
		}

		uint64_t getPlaneSize(size_t planeIdx) const {
			if (planeIdx > format.numPlanes)
				throw std::runtime_error("Pcm plane doesn't exist.");
			return planeSize[planeIdx];
		}

		uint8_t * const * getPlanes() const {
			return planes;
		}

		void setPlanes(uint8_t numAudioPlanes, uint8_t* audioPlanes[AUDIO_PCM_PLANES_MAX], uint64_t audioPlaneSize[AUDIO_PCM_PLANES_MAX]) {
			format.numPlanes = numAudioPlanes;
			freePlanes();
			for (uint8_t i = 0; i < numAudioPlanes; ++i) {
				setPlane(i, planes[i], planeSize[i]);
			}
		}

		void setPlane(uint8_t planeIdx, uint8_t *plane, uint64_t size) {
			if (planeIdx > format.numPlanes)
				throw std::runtime_error("Pcm plane doesn't exist.");
			if ((planes[planeIdx] == nullptr) ||
			        (plane != planes[planeIdx]) ||
			        ((plane == planes[planeIdx]) && (size > planeSize[planeIdx]))) {
				freePlane(planeIdx);
				planes[planeIdx] = new uint8_t[(size_t)size];
			}
			planeSize[planeIdx] = size;
			if (plane && (plane != planes[planeIdx])) {
				memcpy(planes[planeIdx], plane, (size_t)size);
			}
		}

	private:
		void freePlane(uint8_t planeIdx) {
			delete [] planes[planeIdx];
			planes[planeIdx] = nullptr;
			planeSize[planeIdx] = 0;
		}
		void freePlanes() {
			for (uint8_t i = 0; i < format.numPlanes; ++i) {
				freePlane(i);
			}
		}

		PcmFormat format;
		uint8_t* planes[AUDIO_PCM_PLANES_MAX]; //TODO: use std::vector
		uint64_t planeSize[AUDIO_PCM_PLANES_MAX];
};

typedef OutputDataDefault<DataPcm> OutputPcm;

}
