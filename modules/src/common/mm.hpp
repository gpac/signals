#pragma once

#include <cstdint>

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
};

