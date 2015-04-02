#pragma once

#include "lib_modules/core/clock.hpp"
#include "lib_modules/core/module.hpp"
#include "../common/pcm.hpp"
#include "fifo.hpp"
#include <mutex>
#include <memory.h>

struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;

namespace Modules {
namespace Render {

class SDLAudio : public Module {
public:
	static SDLAudio* create(IClock* clock = g_DefaultClock);
	~SDLAudio();
	void process(std::shared_ptr<const Data> data) override;

private:
	SDLAudio(IClock* clock);
	bool reconfigure(PcmFormat const * const pcmFormat);
	static void staticFillAudio(void *udata, uint8_t *stream, int len);
	void fillAudio(uint8_t *stream, int len);

	uint64_t fifoSamplesToRead() const {
		return m_Fifo.bytesToRead() / bytesPerSample;
	}

	void fifoConsumeSamples(size_t n) {
		m_Fifo.consume(n * bytesPerSample);
		m_FifoTime += (n * IClock::Rate) / pcmFormat->sampleRate;
	}

	void writeSamples(uint8_t*& dst, uint8_t const* src, size_t n) {
		memcpy(dst, src, n * bytesPerSample);
		dst += n * bytesPerSample;
	}

	void silenceSamples(uint8_t*& dst, size_t n) {
		memset(dst, 0, n * bytesPerSample);
		dst += n * bytesPerSample;
	}

	IClock* const m_clock;
	static auto const audioJitterTolerance = 500;
	uint8_t bytesPerSample;
	std::unique_ptr<PcmFormat> pcmFormat;
	std::mutex m_Mutex;
	Fifo m_Fifo;
	uint64_t m_FifoTime;
	uint64_t m_Latency;
};

}
}
