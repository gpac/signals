#pragma once

#include "allocator.hpp"
#include "data.hpp"
#include "props.hpp"
#include "../modules/internal/utils/helper.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include <../signals/signals.hpp>
#include <thread>


namespace Modules {

using namespace Signals;

//FIXME: to be removed when modules and mm are clearly separated. In this case the result may not even need to be checked.
typedef Signal<void(std::shared_ptr<Data>), ResultQueueThreadSafe<NotVoid<void>>> SignalAsync;
typedef Signal<void(std::shared_ptr<Data>), ResultVector<NotVoid<void>>> SignalSync;

template<typename Allocator, typename Signal> class PinT;
template<typename DataType> using PinDataAsync = PinT<PacketAllocator<DataType>, SignalAsync>;
template<typename DataType> using PinDataSync = PinT<PacketAllocator<DataType>, SignalSync>;
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
	virtual void setProps(IProps *props) = 0; /*TODO: attached with every emitted packet. shared_data<>?*/
	virtual ISignal<void(std::shared_ptr<Data>)>& getSignal() = 0;
};

inline size_t ConnectPin(Pin* p, std::function<void(std::shared_ptr<Data>)> functor) {
return p->getSignal().connect(functor);
}

template<typename C, typename D>
size_t ConnectPin(Pin* p, C ObjectSlot, D MemberFunctionSlot) {
auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
return ConnectPin(p, functor);
}

inline size_t ConnectPin(Pin* p, std::function<void(std::shared_ptr<Data>)> functor, IProcessExecutor& executor) {
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
	Pin* createPin(IProps *props = nullptr);
};

//TODO: the pin could check the bool result (currently done by the allocator) and retry on failure (but is it its role?)
template<typename Allocator, typename Signal>
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
		if (numReceivers == 0) {
			Log::msg(Log::Debug, "emit(): Pin had no receiver");
		}
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

	/**
	 * Takes ownership.
	 */
	void setProps(IProps *props) {
		this->props = uptr(props);
	}

private:
	Allocator allocator;
	Signal signal;
	std::unique_ptr<IProps> props;
};

}
