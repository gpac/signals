#pragma once

#include "internal/core/clock.hpp"
#include "internal/core/module.hpp"
#include "fifo.hpp"
#include <mutex>
#include <string>
#include <vector>
#include <memory.h>

struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;

namespace Modules {
namespace Render {

//FIXME: check it doesn't need to run in thread 0, like most render on Unix do because of X11...
class SDLAudio : public Module {
public:
	static SDLAudio* create();
	~SDLAudio();
	bool process(std::shared_ptr<Data> data);

private:
	SDLAudio();
	static void staticFillAudio(void *udata, uint8_t *stream, int len);
	void fillAudio(uint8_t *stream, int len);

	uint64_t fifoSamplesToRead() const {
		return m_Fifo.bytesToRead() / bytesPerSample;
	}

	void fifoConsumeSamples(size_t n) {
		m_Fifo.consume(n * bytesPerSample);
		m_FifoTime += (n * IClock::Rate) / AUDIO_SAMPLERATE;
	}

	void writeSamples(uint8_t*& dst, uint8_t const* src, size_t n) {
		memcpy(dst, src, n * bytesPerSample);
		dst += n * bytesPerSample;
	}

	void silenceSamples(uint8_t*& dst, size_t n) {
		memset(dst, 0, n * bytesPerSample);
		dst += n * bytesPerSample;
	}

	static auto const bytesPerSample = 4;
	static auto const audioJitterTolerance = 300;
	std::mutex m_Mutex;
	Fifo m_Fifo;
	uint64_t m_FifoTime;
	uint64_t m_Latency;
};

}
}
