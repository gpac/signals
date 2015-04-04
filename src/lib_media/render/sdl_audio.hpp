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
	SDLAudio(IClock* clock = g_DefaultClock);
	~SDLAudio();
	void process(std::shared_ptr<const Data> data) override;

private:
	bool reconfigure(PcmFormat const * const pcmFormat);
	void push(std::shared_ptr<const Data> data);
	static void staticFillAudio(void *udata, uint8_t *stream, int len);
	void fillAudio(uint8_t *stream, int len);

	uint64_t fifoSamplesToRead() const;
	void fifoConsumeSamples(size_t n);
	void writeSamples(uint8_t*& dst, uint8_t const* src, size_t n);
	void silenceSamples(uint8_t*& dst, size_t n);

	IClock* const m_clock;
	static auto const audioJitterTolerance = 500;
	uint8_t bytesPerSample;
	std::unique_ptr<const PcmFormat> pcmFormat;
	std::unique_ptr<Module> m_converter;
	std::mutex m_Mutex;
	Fifo m_Fifo;
	uint64_t m_FifoTime;
	uint64_t m_Latency;
};

}
}
