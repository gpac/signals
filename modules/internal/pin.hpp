#pragma once

#include "allocator.hpp"
#include "data.hpp"
#include <../signals/signals.hpp>
#include <thread>


namespace Modules {

using namespace Signals;

//TODO: this is the sync approach, where data are synced for the Pin to be destroyed.
//      The other option is to invalidate all the data by calling
//TODO: the pin could check the bool result and retry on failure (but is it its role?)
template<typename Allocator = AllocatorPacket, typename Signal = Signal<bool(std::shared_ptr<Data>), ResultVector<bool>, CallerSync>/*<bool(std::shared_ptr<Data>)>*/>
class MODULES_EXPORT Pin {
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
				signal.results(false); //see if results are ready
				data = allocator.getBuffer(size);
				if (data.get()) {
					return data;
				} else {
					signal.results(true, true); //wait synchronously for one result
					data = allocator.getBuffer(size);
					if (data.get()) {
						return data;
					} else {
						std::this_thread::yield(); //FIXME
					}
				}
			}
		}
	}

	Signal signal;

private:
	Allocator allocator;
};

typedef Pin<AllocatorPacket, Signal<bool(std::shared_ptr<Data>), ResultVector<bool>, CallerSync>> PinSync;

}
