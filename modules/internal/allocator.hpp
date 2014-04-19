#pragma once

#include "config.hpp"
#include "data.hpp"
#include <algorithm>
#include <atomic>
#include <list>
#include <memory>

namespace Modules {

#define COUNT_ALLOC

//TODO: this is a (blocking) allocator by packets - write a more generalist one
//TODO: make a non-blocking one by calling reset() on the shared_ptr - may required appropriate checks in the modules
//FIXME: not thread-safe, so cannot be shared between modules
template<typename DataType>
class MODULES_EXPORT AllocatorPacket {
public:
	AllocatorPacket(size_t numBlocks = 10)
		: numBlocks(numBlocks)
#ifdef COUNT_ALLOC
		, numAlloc(0)
#endif
	{
	}

	std::shared_ptr<DataType> getBuffer(size_t size, bool forceNew = false) {
		updateUsedBlocks();
		if (usedBlocks.size() < numBlocks) {
			std::shared_ptr<DataType> data(new DataType(size));
			usedBlocks.push_back(std::weak_ptr<DataType>(data));
#ifdef COUNT_ALLOC
			numAlloc++;
#endif
			return data;
		} else {
			if (forceNew) {
				numBlocks++;
				std::shared_ptr<DataType> data(new DataType(size));
				usedBlocks.push_back(std::weak_ptr<DataType>(data));
#ifdef COUNT_ALLOC
				numAlloc++;
#endif
				return data;
			} else {
				return std::shared_ptr<DataType>();
			}
		}
	}

	size_t getNumBlocks() const {
		return numBlocks;
	}

	size_t getNumUsedBlocks() const {
		return usedBlocks.size();
	}

#ifdef COUNT_ALLOC
	uint64_t getNumAlloc() const {
		return numAlloc;
	}
#endif

	void reset() {
		for (auto &block : usedBlocks) {
			block.reset();
		}
		updateUsedBlocks();
	}

	void updateUsedBlocks() {
		usedBlocks.erase(std::remove_if(usedBlocks.begin(), usedBlocks.end(),
		[](std::weak_ptr<Data> data) -> bool {
			return data.expired();
		}), usedBlocks.end());
	}

private:
	AllocatorPacket& operator= (const AllocatorPacket&) = delete;

	size_t numBlocks;
	std::list<std::weak_ptr<Data>> usedBlocks;
#ifdef COUNT_ALLOC
	std::atomic<uint64_t> numAlloc;
#endif
};

}
