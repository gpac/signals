#pragma once

#include <cstdint>
#include <string>
#include "lib_modules/core/data.hpp"
#include "lib_modules/core/metadata.hpp"

struct AVCodecContext;

namespace Modules {

enum StreamType {
	UNKNOWN_ST = -1,
	PCM,       //UNCOMPRESSED_AUDIO
	PICTURE,   //UNCOMPRESSED_VIDEO
	AUDIO_PKT, //COMPRESSED_AUDIO
	VIDEO_PKT  //COMPRESSED_VIDEO
};

class StreamVideo : public Modules::Data, public IProperty {
public:
	uint32_t width;
	uint32_t height;
	uint32_t timeScale;
	const uint8_t *extradata; //TODO: who holds this? std::vector?
	size_t extradataSize;

	AVCodecContext *codecCtx; //FIXME: legacy from libav
};

class StreamAudio : public Modules::Data, public IProperty {
public:
	std::string codecName;
	uint32_t numChannels;
	uint32_t sampleRate;
	uint8_t bitsPerSample;
	uint32_t frameSize;
	const uint8_t *extradata; //TODO: who holds this? std::vector?
	size_t extradataSize;

	AVCodecContext *codecCtx; //FIXME: legacy from libav
};
}
