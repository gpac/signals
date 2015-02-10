#pragma once

#include "data.hpp"
#include "lib_signals/internal/utils/queue.hpp"
#include <algorithm>
#include <list>
#include <memory>

namespace Modules {

template<typename DataType>
class PacketAllocator {
public:
	typedef DataType MyType;
	PacketAllocator(size_t numBlocks = 10)
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

	std::shared_ptr<DataType> getBuffer(size_t size) {
		auto event = freeBlocks.pop(); 
		switch(event) {
		case OneBufferIsFree: {
			std::shared_ptr<DataType> data(new DataType(size), deleter);
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
