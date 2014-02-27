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

template<typename Allocator, typename Signal> class PinT;
typedef MODULES_EXPORT PinT<AllocatorPacket, SignalAsync> PinAsync;
typedef MODULES_EXPORT PinT<AllocatorPacket, SignalSync> PinSync;
typedef MODULES_EXPORT PinSync PinDefault;

class Pin {
public:
	virtual ~Pin() {
	}
	virtual size_t emit(std::shared_ptr<Data> data) = 0;
	virtual void waitForCompletion() = 0;
	virtual std::shared_ptr<Data> getBuffer(size_t size) = 0;
	virtual Props* getProps() const = 0;
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
	virtual Pin* createPin(Props *props = nullptr) = 0;
};

class PinDefaultFactory : public PinFactory {
public:
	PinDefaultFactory();
	Pin* createPin(Props *props = nullptr);
};

}
