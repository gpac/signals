#pragma once

#include "data.hpp"
#include "../../../signals/internal/utils/queue.hpp"
#include <algorithm>
#include <list>
#include <memory>

namespace Modules {

template<typename DataType>
class AllocatorPacket {
public:
	AllocatorPacket(size_t numBlocks = 10)
		: deleter(this) {
		for(size_t i=0;i < numBlocks; ++i) {
			freeBlocks.push(false);
		}
	}

	struct Deleter
	{
		Deleter(AllocatorPacket<DataType>* allocator) : m_allocator(allocator)
		{
		}

		void operator()(DataType* p)
		{
			m_allocator->recycle(p);
		}

		AllocatorPacket<DataType>* const m_allocator;
	};

	std::shared_ptr<DataType> getBuffer(size_t size) {
		auto eos = freeBlocks.pop();
		if(eos) {
			return nullptr;
		}
		std::shared_ptr<DataType> data(new DataType(size), deleter);
		return data;
	}

	void flush() {
		freeBlocks.push(true);
	}

private:
	AllocatorPacket& operator= (const AllocatorPacket&) = delete;

	Deleter deleter;
	Signals::QueueThreadSafe<bool> freeBlocks;

	void recycle(DataType* p) {
		delete p;
		freeBlocks.push(false);
	}

};

}
