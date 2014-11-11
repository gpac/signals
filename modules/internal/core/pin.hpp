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
typedef Signal<void(std::shared_ptr<Data>), ResultQueue<NotVoid<void>>> SignalAsync;
typedef Signal<void(std::shared_ptr<Data>), ResultVector<NotVoid<void>>> SignalSync;

template<typename Allocator, typename Signal> class PinT;
template<typename DataType> using PinDataAsync = PinT<PacketAllocator<DataType>, SignalAsync>;
template<typename DataType> using PinDataSync = PinT<PacketAllocator<DataType>, SignalSync>;
template<typename DataType> using PinDataDefault = PinDataSync<DataType>;

typedef PinDataDefault<RawData> PinDefault;

struct IPin {
	virtual ~IPin() {
	}
	virtual size_t emit(std::shared_ptr<Data> data) = 0;
	virtual std::shared_ptr<Data> getBuffer(size_t size) = 0;
	virtual IProps* getProps() const = 0;
	virtual void setProps(IProps *props) = 0; /*TODO: attached with every emitted packet. shared_data<>?*/
	virtual ISignal<void(std::shared_ptr<Data>)>& getSignal() = 0;
};

inline size_t ConnectPin(IPin* p, std::function<void(std::shared_ptr<Data>)> functor) {
return p->getSignal().connect(functor);
}

template<typename C, typename D>
size_t ConnectPin(IPin* p, C ObjectSlot, D MemberFunctionSlot) {
auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
return ConnectPin(p, functor);
}

inline size_t ConnectPin(IPin* p, std::function<void(std::shared_ptr<Data>)> functor, IProcessExecutor& executor) {
return p->getSignal().connect(functor, executor);
}

template<typename C, typename D, typename E>
size_t ConnectPin(IPin* p, C ObjectSlot, D MemberFunctionSlot, E& executor) {
auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
return ConnectPin(p, functor, executor);
}

class PinDefaultFactory {
public:
	IPin* createPin(IProps *props = nullptr);
};

//TODO: the pin could check the bool result (currently done by the allocator) and retry on failure (but is it its role?)
template<typename Allocator, typename Signal>
class PinT : public IPin {
public:
	PinT(IProps *props = nullptr)
		: props(props) {
	}

	~PinT() {
		allocator.unblock();
	}

	size_t emit(std::shared_ptr<Data> data) {
		size_t numReceivers = signal.emit(data);
		if (numReceivers == 0) {
			Log::msg(Log::Debug, "emit(): Pin had no receiver");
		}
		return numReceivers;
	}

	std::shared_ptr<Data> getBuffer(size_t size) {
		return allocator.getBuffer(size);
	}

	ISignal<void(std::shared_ptr<Data>)>& getSignal() override {
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
