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

typedef Signal<void(std::shared_ptr<const Data>), ResultQueue<NotVoid<void>>> SignalAsync;
typedef Signal<void(std::shared_ptr<const Data>), ResultVector<NotVoid<void>>> SignalSync;

typedef SignalSync SignalDefaultSync;

struct IPin {
	virtual ~IPin() {
	}
	virtual size_t emit(std::shared_ptr<const Data> data) = 0;
	virtual IProps* getProps() const = 0;
	virtual void setProps(IProps *props) = 0; /*TODO: attached with every emitted packet. shared_data<>?*/
	virtual ISignal<void(std::shared_ptr<const Data>)>& getSignal() = 0;
};

inline size_t ConnectPin(IPin* p, std::function<void(std::shared_ptr<const Data>)> functor) {
	return p->getSignal().connect(functor);
}

template<typename C, typename D>
size_t ConnectPin(IPin* p, C ObjectSlot, D MemberFunctionSlot) {
	auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
	return ConnectPin(p, functor);
}

inline size_t ConnectPin(IPin* p, std::function<void(std::shared_ptr<const Data>)> functor, IProcessExecutor& executor) {
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

	size_t emit(std::shared_ptr<const Data> data) {
		size_t numReceivers = signal.emit(data);
		if (numReceivers == 0) {
			Log::msg(Log::Debug, "emit(): Pin had no receiver");
		}
		return numReceivers;
	}

	std::shared_ptr<typename Allocator::MyType> getBuffer(size_t size) {
		return allocator.getBuffer(size);
	}

	ISignal<void(std::shared_ptr<const Data>)>& getSignal() override {
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

template<typename DataType> using PinDataDefault = PinT<PacketAllocator<DataType>, SignalDefaultSync>;

typedef PinDataDefault<RawData> PinDefault;

}
