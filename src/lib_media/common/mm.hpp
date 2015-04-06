#pragma once

#include <cstdint>
#include <string>
#include "lib_modules/core/data.hpp"

struct AVCodecContext;

class Stream : public Modules::Data {
public:
	virtual ~Stream() {};
};

class StreamVideo : public Stream {
public:
	uint32_t width;
	uint32_t height;
	uint32_t timeScale;
	const uint8_t *extradata; //TODO: who holds this? std::vector?
	size_t extradataSize;

	AVCodecContext *codecCtx; //FIXME: legacy from libav
};

class StreamAudio : public Stream {
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

