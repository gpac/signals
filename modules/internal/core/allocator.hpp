#pragma once

#include "data.hpp"
#include <algorithm>
#include <list>
#include <memory>

namespace Modules {

//#define COUNT_ALLOC

//TODO: this is a (blocking) allocator by packets - write a more generalist one
//TODO: make a non-blocking one by calling reset() on the shared_ptr - may required appropriate checks in the modules
//FIXME: not thread-safe, so cannot be shared between modules
template<typename DataType>
class AllocatorPacket {
public:
	AllocatorPacket(size_t numBlocks = 10)
		: numBlocks(numBlocks)
	{
	}

	std::shared_ptr<DataType> getBuffer(size_t size, bool forceNew = false) {
		updateUsedBlocks();
		if (usedBlocks.size() < numBlocks) {
			std::shared_ptr<DataType> data(new DataType(size));
			usedBlocks.push_back(std::weak_ptr<DataType>(data));
			return data;
		}

		if (forceNew) {
			numBlocks++;
			std::shared_ptr<DataType> data(new DataType(size));
			usedBlocks.push_back(std::weak_ptr<DataType>(data));
			return data;
		}
		
		return nullptr;
	}

	size_t getNumUsedBlocks() {
		updateUsedBlocks();
		return usedBlocks.size();
	}

	void reset() {
		for (auto &block : usedBlocks) {
			block.reset();
		}
		updateUsedBlocks();
	}

private:
	AllocatorPacket& operator= (const AllocatorPacket&) = delete;

	size_t numBlocks;
	std::list<std::weak_ptr<Data>> usedBlocks;

	void updateUsedBlocks() {
		auto isFree = [](std::weak_ptr<Data> data) {
			return data.expired();
		};
			
		usedBlocks.erase(std::remove_if(usedBlocks.begin(), usedBlocks.end(), isFree), usedBlocks.end());
	}

};

}
