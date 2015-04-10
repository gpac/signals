#pragma once

#include "data.hpp"
#include "lib_signals/internal/utils/queue.hpp"
#include <algorithm>
#include <list>
#include <memory>

namespace Modules {

static const size_t ALLOC_NUM_BLOCKS_DEFAULT = 10;
static const size_t ALLOC_NUM_BLOCKS_LOW_LATENCY = 1;

template<typename DataType>
class PacketAllocator {
public:
	typedef DataType MyType;
	PacketAllocator(size_t numBlocks = ALLOC_NUM_BLOCKS_DEFAULT)
		: deleter(this) {
		for(size_t i=0; i < numBlocks; ++i) {
			freeBlocks.push(OneBufferIsFree);
		}
	}

	struct Deleter {
		Deleter(PacketAllocator<DataType>* allocator) : m_allocator(allocator) {
		}
		void operator()(DataType* p) {
			m_allocator->recycle(p);
		}
		PacketAllocator<DataType>* const m_allocator;
	};

	template<typename T>
	std::shared_ptr<T> getBuffer(size_t size) {
		auto event = freeBlocks.pop(); 
		switch(event) {
		case OneBufferIsFree: {
			std::shared_ptr<T> data(new T(size), deleter);
			return data;
			}
		case Exit:
			return nullptr;
		}
		return nullptr;
	}

	void unblock() {
		freeBlocks.push(Exit);
	}

private:
	PacketAllocator& operator= (const PacketAllocator&) = delete;

	enum Event {
		OneBufferIsFree,
		Exit,
	};

	Deleter deleter;
	Signals::Queue<Event> freeBlocks;

	void recycle(DataType* p) {
		delete p;
		freeBlocks.push(OneBufferIsFree);
	}
};

}
