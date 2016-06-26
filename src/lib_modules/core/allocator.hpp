#pragma once

#include "data.hpp"
#include "lib_signals/utils/queue.hpp"
#include <algorithm>
#include <list>
#include <memory>
#include <stdexcept>

namespace Modules {

static const size_t ALLOC_NUM_BLOCKS_DEFAULT = 10;
static const size_t ALLOC_NUM_BLOCKS_LOW_LATENCY = 2;

template<typename DataType>
class PacketAllocator {
	public:
		typedef DataType MyType;
		PacketAllocator(size_t numBlocks) {
			if (numBlocks == 0)
				throw std::runtime_error("Cannot create an allocator with 0 block.");
			for(size_t i=0; i < numBlocks; ++i) {
				freeBlocks.push(Block());
			}
		}

		struct Deleter {
			Deleter(PacketAllocator<DataType> *allocator) : allocator(allocator) {}
			void operator()(DataType *p) const {
				allocator->recycle(p);
			}
			PacketAllocator<DataType> * const allocator;
		};

		template<typename T>
		std::shared_ptr<T> getBuffer(size_t size) {
			auto block = freeBlocks.pop();
			switch(block.event) {
			case OneBufferIsFree: {
				if (!block.data) {
					delete block.data;
					block.data = new T(size);
				} else {
					auto data = safe_cast<T>(block.data);
					if (data->size() < size)
						data->resize(size);
				}

				return std::shared_ptr<T>(safe_cast<T>(block.data), Deleter(this));
			}
			case Exit:
				return nullptr;
			}
			return nullptr;
		}

		void unblock() {
			freeBlocks.push(Block(Exit));
		}

	private:
		PacketAllocator& operator= (const PacketAllocator&) = delete;
		void recycle(DataType *p) {
			if (!p->isRecyclable()) {
				delete p;
				p = nullptr;
			}
			freeBlocks.push(Block(OneBufferIsFree, p));
		}

		enum Event {
			OneBufferIsFree,
			Exit,
		};
		struct Block {
			Block(Event event = OneBufferIsFree, DataType *data = nullptr): event(event), data(data) {}
			Event event;
			DataType *data;
		};

		Signals::Queue<Block> freeBlocks;
};

}
