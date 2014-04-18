#pragma once

#include "config.hpp"
#include "data.hpp"
#include <algorithm>
#include <list>
#include <memory>

namespace Modules {

//TODO: this is a (blocking) allocator by packets - write a more generalist one
//TODO: make a non-blocking one by calling reset() on the shared_ptr - may required appropriate checks in the modules
//FIXME: not thread-safe, so cannot be shared between modules
template<typename DataType>
class MODULES_EXPORT AllocatorPacket {
public:
	AllocatorPacket(size_t numBlocks = 10)
		: numBlocks(numBlocks) {
	}

	std::shared_ptr<DataType> getBuffer(size_t size, bool forceNew = false) {
		updateUsedBlocks();
		if (usedBlocks.size() < numBlocks) {
			std::shared_ptr<DataType> data(new DataType(size));
			usedBlocks.push_back(std::weak_ptr<DataType>(data));
			return data;
		} else {
			if (forceNew) {
				numBlocks++;
				std::shared_ptr<DataType> data(new DataType(size));
				usedBlocks.push_back(std::weak_ptr<DataType>(data));
				return data;
			} else {
				return std::shared_ptr<DataType>();
			}
		}
	}

	void reset() {
		for (auto &block : usedBlocks) {
			block.reset();
		}
	}

private:
	AllocatorPacket& operator= (const AllocatorPacket&) = delete;

	void updateUsedBlocks() {
		usedBlocks.erase(std::remove_if(usedBlocks.begin(), usedBlocks.end(),
		[](std::weak_ptr<Data> data) -> bool {
			return data.expired();
		}), usedBlocks.end());
	}

	size_t numBlocks;
	std::list<std::weak_ptr<Data>> usedBlocks;
};

}
