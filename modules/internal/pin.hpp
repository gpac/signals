#pragma once

#include "config.hpp"
#include "allocator.hpp"
#include "data.hpp"
#include "props.hpp"
#include <../signals/signals.hpp>
#include <thread>


namespace Modules {

using namespace Signals;

//TODO: the pin could check the bool result (currently done by the allocator) and retry on failure (but is it its role?)
template<typename Allocator, typename Signal>
class PinT {
public:
	typedef Signal SignalType;

	PinT(Props *props = nullptr)
	: props(props) {		
	}

	~PinT() {
		destroy();
	}

	size_t emit(std::shared_ptr<Data> data) {
		size_t numReceivers = signal.emit(data);
		assert(numReceivers == 1);
		return numReceivers;
	}

	void destroy() {
		signal.results(); //getting the result release the shared_ptr
	}

	//TODO: this is the sync approach, where data are synced for the Pin to be destroyed.
	//      The other option is to invalidate all the data by calling
	std::shared_ptr<Data> getBuffer(size_t size) {
		for(;;) {
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

	Signal& getSignal() {
		return signal;
	}

	Props* getProps() const {
		return props.get();
	}

private:
	Allocator allocator;
	Signal signal;
	std::unique_ptr<Props> props;
};

typedef MODULES_EXPORT PinT<AllocatorPacket, Signal<bool(std::shared_ptr<Data>), ResultQueueThreadSafe<bool>, CallerAsync>> PinAsync;
typedef MODULES_EXPORT PinT<AllocatorPacket, Signal<bool(std::shared_ptr<Data>), ResultVector<bool>, CallerSync>> PinSync;
typedef MODULES_EXPORT PinSync Pin; //FIXME: all pins are sync right now...

}
