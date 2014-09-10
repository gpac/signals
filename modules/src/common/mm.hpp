#pragma once

#include <cstdint>
#include <string>

struct AVCodecContext;

class Stream {
public:
	virtual ~Stream() {};
};

class StreamVideo : public Stream {
public:
	uint32_t width;
	uint32_t height;
	uint32_t timeScale;
	const uint8_t *extradata; //TODO: who holds this? std::vector?
	uint64_t extradataSize;

	AVCodecContext *codecCtx; //FIXME: legacy from libav
};

class StreamAudio : public Stream {
public:
	std::string codecName;
	uint32_t numChannels;
	uint32_t sampleRate;
	uint8_t bitsPerSample;
	const uint8_t *extradata; //TODO: who holds this? std::vector?
	uint64_t extradataSize;

	AVCodecContext *codecCtx; //FIXME: legacy from libav
};

