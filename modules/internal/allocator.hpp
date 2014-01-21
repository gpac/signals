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
class MODULES_EXPORT AllocatorPacket {
public:
	AllocatorPacket(size_t numBlocks = 10)
		: numBlocks(numBlocks) {
	}

	std::shared_ptr<Data> getBuffer(size_t size) {
		updateUsedBlocks();
		if (usedBlocks.size() < numBlocks) {
			std::shared_ptr<Data> data(new Data(size));
			usedBlocks.push_back(std::weak_ptr<Data>(data));
			return (data);
		} else {
			return std::shared_ptr<Data>();
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

	const size_t numBlocks;
	std::list<std::weak_ptr<Data>> usedBlocks;
};

}
