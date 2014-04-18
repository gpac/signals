#pragma once

#include "config.hpp"
#include "allocator.hpp"
#include "data.hpp"
#include "props.hpp"
#include "../utils/log.hpp"
#include <../signals/signals.hpp>
#include <thread>


namespace Modules {

using namespace Signals;

typedef Signal<bool(std::shared_ptr<Data>), ResultQueueThreadSafe<bool>, CallerAsync> SignalAsync;
typedef Signal<bool(std::shared_ptr<Data>), ResultVector<bool>, CallerSync> SignalSync;

template<typename Allocator, typename Signal, typename DataType> class PinT;
template<typename DataType> using PinDataAsync = PinT<AllocatorPacket<DataType>, SignalAsync, DataType>;
template<typename DataType> using PinDataSync = PinT<AllocatorPacket<DataType>, SignalSync, DataType>;
template<typename DataType> using PinDataDefault = PinDataSync<DataType>;

typedef MODULES_EXPORT PinDataAsync<Data> PinAsync;
typedef MODULES_EXPORT PinDataSync<Data> PinSync;
typedef MODULES_EXPORT PinDataDefault<Data> PinDefault;

class Pin {
public:
	virtual ~Pin() {
	}
	virtual size_t emit(std::shared_ptr<Data> data) = 0;
	virtual void waitForCompletion() = 0;
	virtual std::shared_ptr<Data> getBuffer(size_t size) = 0;
	virtual IProps* getProps() const = 0;
	virtual ISignal<bool(std::shared_ptr<Data>)>& getSignal() = 0;
};

template<typename C, typename D>
void ConnectPin(Pin* p, C ObjectSlot, D MemberFunctionSlot) {
	auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
	p->getSignal().connect(functor);
}


class PinFactory {
public:
	virtual ~PinFactory() {
	}
	virtual Pin* createPin(IProps *props = nullptr) = 0;
};

class PinDefaultFactory : public PinFactory {
public:
	PinDefaultFactory();
	Pin* createPin(IProps *props = nullptr);
};

//TODO: the pin could check the bool result (currently done by the allocator) and retry on failure (but is it its role?)
template<typename Allocator, typename Signal, typename DataType>
class PinT : public Pin {
public:
	PinT(IProps *props = nullptr)
		: props(props) {
	}

	~PinT() {
		waitForCompletion();
	}

	size_t emit(std::shared_ptr<Data> data) {
		size_t numReceivers = signal.emit(data);
		assert(numReceivers == 1);
		return numReceivers;
	}

	void waitForCompletion() {
		signal.results(); //getting the result release the shared_ptr
	}

	//TODO: this is the sync approach, where data are synced for the Pin to be destroyed.
	//      The other option is to invalidate all the data by calling
	std::shared_ptr<Data> getBuffer(size_t size) {
		for (;;) {
			std::shared_ptr<DataType> data(allocator.getBuffer(size));
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
#if 0
						Log::msg(Log::Error, "The allocator failed to wait for available data. Reset the whole allocator.");
						allocator.reset();
						data = allocator.getBuffer(size);
#else
						Log::msg(Log::Error, "The allocator failed to wait for available data. Add a new buffer.");
						data = allocator.getBuffer(size, true);
#endif
						assert(data.get());
						return data;
					}
				}
			}
		}
	}

	Signal& getSignal() {
		return signal;
	}

	IProps* getProps() const {
		return props.get();
	}

private:
	Allocator allocator;
	Signal signal;
	std::unique_ptr<IProps> props;
};

}
