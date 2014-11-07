#pragma once

#include "data.hpp"
#include "../../../signals/internal/utils/queue.hpp"
#include <algorithm>
#include <list>
#include <memory>

namespace Modules {

template<typename DataType>
class PacketAllocator {
public:
	PacketAllocator(size_t numBlocks = 10)
		: deleter(this) {
		for(size_t i=0; i < numBlocks; ++i) {
			freeBlocks.push(false);
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
		auto eos = freeBlocks.tryPop();
		if (eos) {
			return nullptr;
		}
		std::shared_ptr<DataType> data(new DataType(size), deleter);
		return data;
	}

	void flush() {
		freeBlocks.push(true);
	}

private:
	PacketAllocator& operator= (const PacketAllocator&) = delete;

	Deleter deleter;
	Signals::QueueThreadSafe<bool> freeBlocks;

	void recycle(DataType* p) {
		delete p;
		freeBlocks.push(false);
	}

};

}
