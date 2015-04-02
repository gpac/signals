#pragma once
#include <stdint.h>
#include <memory.h>

template<typename T>
class GenericFifo {
public:
	GenericFifo() : m_writePos(0), m_readPos(0) {
	}

	void write(const T* data, size_t len) {
		if (!len) return;
		m_data.resize(m_writePos + len); //Romain: the pb may come from multiple pushes here?
		memcpy(&m_data[m_writePos], data, len);
		m_writePos += len;
	}

	const T* readPointer() {
		return &m_data[m_readPos];
	}

	void consume(size_t numBytes) {
		assert(numBytes >= 0);
		assert(numBytes <= bytesToRead());
		m_readPos += numBytes;

		// shift everything to the beginning of the buffer
		memmove(m_data.data(), m_data.data() + m_readPos, bytesToRead());
		m_writePos -= m_readPos;
		m_readPos = 0;
	}

	size_t bytesToRead() const {
		return m_writePos - m_readPos;
	}

private:
	size_t m_writePos;
	size_t m_readPos;
	std::vector<T> m_data;
};

typedef GenericFifo<uint8_t> Fifo;

