#pragma once

#include "allocator.hpp"
#include "data.hpp"
#include "props.hpp"
#include "../utils/log.hpp"
#include <../signals/signals.hpp>
#include <thread>


namespace Modules {

using namespace Signals;

//FIXME: to be removed when modules and mm are clearly separated. In this case the result may not even need to be checked.
typedef Signal<bool(std::shared_ptr<Data>), ResultQueueThreadSafe<bool>> SignalAsync;
typedef Signal<bool(std::shared_ptr<Data>), ResultVector<bool>> SignalSync;

template<typename Allocator, typename Signal, typename DataType> class PinT;
template<typename DataType> using PinDataAsync = PinT<AllocatorPacket<DataType>, SignalAsync, DataType>;
template<typename DataType> using PinDataSync = PinT<AllocatorPacket<DataType>, SignalSync, DataType>;
template<typename DataType> using PinDataDefault = PinDataSync<DataType>;

typedef PinDataAsync<Data> PinAsync;
typedef PinDataSync<Data> PinSync;
typedef PinDataDefault<Data> PinDefault;

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

inline size_t ConnectPin(Pin* p, std::function<bool(std::shared_ptr<Data>)> functor) {
	return p->getSignal().connect(functor);
}

template<typename C, typename D>
size_t ConnectPin(Pin* p, C ObjectSlot, D MemberFunctionSlot) {
	auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
	return ConnectPin(p, functor);
}

inline size_t ConnectPin(Pin* p, std::function<bool(std::shared_ptr<Data>)> functor, IExecutor<bool(std::shared_ptr<Data>)>& executor) {
	return p->getSignal().connect(functor, executor);
}

template<typename C, typename D, typename E>
size_t ConnectPin(Pin* p, C ObjectSlot, D MemberFunctionSlot, E& executor) {
	auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
	return ConnectPin(p, functor, executor);
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
		assert(numReceivers >= 1);
		return numReceivers;
	}

	void waitForCompletion() {
		//FIXME: we have to wait for all samples to be processed. Waiting can be long because the allocator doesn't signal
		// and creates as much buffers as needed. If it waited, we could have better performance results and event-based waiting here.
		const int sleepDurInMs = 10;
		const std::chrono::milliseconds dur(sleepDurInMs);
		int maxSleep = 3000;

		size_t usedBlocks = tryFlushAllocator();
		while (usedBlocks && --maxSleep > 0) {
			std::this_thread::sleep_for(dur); //FIXME: we should set events when Data are freed
			usedBlocks = tryFlushAllocator();
		}
		if (maxSleep == 0) {
			Log::msg(Log::Warning, "Warning: force invalidating the allocator data.");
			allocator.reset();
		}
	}

	//TODO: this is the sync approach, where data are synced for the Pin to be destroyed.
	//      The other option is to invalidate all the data by calling
	std::shared_ptr<Data> getBuffer(size_t size) {
		std::shared_ptr<DataType> data(allocator.getBuffer(size));
		if (data.get()) {
			return data;
		}

		signal.results(false); //see if results are ready
		data = allocator.getBuffer(size);
		if (data.get()) {
			return data;
		}

		signal.results(true, true); //wait synchronously for one result
		data = allocator.getBuffer(size);
		if (data.get()) {
			return data;
		}

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

	Signal& getSignal() {
		return signal;
	}

	IProps* getProps() const {
		return props.get();
	}

private:
	size_t tryFlushAllocator() {
		signal.results();                          //getting the result release the future shared_ptr
		auto usedBlocks = allocator.getNumUsedBlocks(); //some blocks may stay if the allocator data is processed by further modules
		return usedBlocks;
	}

	Allocator allocator;
	Signal signal;
	std::unique_ptr<IProps> props;
};

}
