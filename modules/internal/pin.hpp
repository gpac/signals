#pragma once

#include "allocator.hpp"
#include "data.hpp"
#include <../signals/signals.hpp>


//TODO: this is the sync approach, where data are synced for the Pin to be destroyed.
//      The other option is to invalidate all the data by calling
//TODO: the pin could check the bool result and retry on failure (but is it its role?)
template<typename Allocator = AllocatorPacket>
class EXPORT Pin {
public:
	~Pin() {
		destroy();
	}

	size_t emit(std::shared_ptr<Data> data) {
		size_t numReceivers = signal.emit(data);
		assert(numReceivers == 1);
		return numReceivers;
	}

	void destroy() {
		//getting the result release the shared_ptr, hence the data
		//FIXME: we should lock since emit could be called from any thread
		signal.results();
	}

	std::shared_ptr<Data> getBuffer(size_t size) {
		while (1) {
			std::shared_ptr<Data> data(allocator.getBuffer(size));
			if (data.get()) {
				return data;
			} else {
				destroy();
			}
		}
	}

	//Signal<bool(std::shared_ptr<Data>)> signal;
	Signal<bool(std::shared_ptr<Data>), ResultVector<bool>, CallerSync> signal;

private:
	Allocator allocator;
};
