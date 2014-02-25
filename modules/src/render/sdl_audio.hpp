#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <memory.h>

struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;

namespace Modules {
namespace Render {

// TODO: move elsewhere
class Fifo {
public:

	Fifo() : m_writePos(0), m_readPos(0) {
	}

	void write(const uint8_t* data, size_t len) {
		m_data.resize(m_writePos + len);
		memcpy(&m_data[m_writePos], data, len);
		m_writePos += len;
	}

	const uint8_t* readPointer() {
		return &m_data[m_readPos];
	}

	void consume(int numBytes) {
		assert(numBytes >= 0);
		assert(numBytes <= bytesToRead());
		m_readPos += numBytes;

		// shift everything to the beginning of the buffer
		memmove(m_data.data(), m_data.data() + m_readPos, bytesToRead());
		m_writePos -= m_readPos;
		m_readPos = 0;
	}

	int bytesToRead() const {
		return m_writePos - m_readPos;
	}

private:
	size_t m_writePos;
	size_t m_readPos;
	std::vector<uint8_t> m_data;
};

//FIXME: check it doesn't need to run in thread 0, like most render on Unix do because of X11...
class MODULES_EXPORT SDLAudio : public Module {
public:
	static SDLAudio* create();
	~SDLAudio();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	SDLAudio();
	static void staticFillAudio(void *udata, uint8_t *stream, int len);
	void fillAudio(uint8_t *stream, int len);

	std::mutex m_Mutex;
	Fifo m_Fifo;
};

}
}
