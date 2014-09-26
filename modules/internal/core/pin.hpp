#pragma once

#include "allocator.hpp"
#include "data.hpp"
#include "props.hpp"
#include "internal/utils/helpers.hpp"
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

inline size_t ConnectPin(Pin* p, std::function<bool(std::shared_ptr<Data>)> functor, IProcessExecutor& executor) {
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
		allocator.flush();
	}

	std::shared_ptr<Data> getBuffer(size_t size) {
		//FIXME awful hack: infinite loop to wait for available results, otherwise we are blocked
		for (uint64_t i = 0; ; ++i) {
			signal.results(false); //see if results are ready
			auto data = allocator.getBuffer(size);
			if (data.get()) {
				return data;
			}
			const std::chrono::milliseconds dur(10);
			std::this_thread::sleep_for(dur);
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
