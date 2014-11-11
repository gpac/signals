#pragma once

#include "../../../signals/signals.hpp"
#include <memory>
#include <vector>


using namespace Signals;

namespace Modules {

struct IClock {
static auto const Rate = 180000LL;
};

class Data {
public:
	Data(size_t size) : ptr(size) {}
	virtual ~Data() {}
	uint8_t* data() {
		return ptr.data();
	}
	uint64_t size() const {
		return ptr.size();
	}
	void resize(size_t size) {
		ptr.resize(size);
	}
	void setTime(uint64_t timeIn180k) {
		m_TimeIn180k = timeIn180k;
	}
	uint64_t getTime() const {
		return m_TimeIn180k;
	}
	void setDuration(uint64_t DurationIn180k) {
		m_DurationIn180k = DurationIn180k;
	}
	void setDuration(uint64_t DurationInTimescale, uint64_t timescale) {
		m_DurationIn180k = (DurationInTimescale * IClock::Rate + timescale / 2) / timescale;
	}
	uint64_t getDuration() const {
		return m_DurationIn180k;
	}

private:
	std::vector<uint8_t> ptr;
	uint64_t m_TimeIn180k;
	uint64_t m_DurationIn180k;
};


class IProps {
public:
	virtual ~IProps() {}
};

class Pin {
public:
	Pin(IProps *props = nullptr) : props(props) {}
	~Pin() { }
	size_t emit(std::shared_ptr<Data> data) {
		return 1;
	}
	std::shared_ptr<Data> getBuffer(size_t size) {
		return std::shared_ptr<Data>(new Data(size));
	}
	Signal<void(std::shared_ptr<Data>), ResultQueue<NotVoid<void>>>& getSignal() {
		return signal;
	}
	IProps* getProps() const {
		return props.get();
	}

private:
	Signal<void(std::shared_ptr<Data>), ResultQueue<NotVoid<void>>> signal;
	std::unique_ptr<IProps> props;
};

class PinFactory {
public:
	virtual ~PinFactory() {}
	virtual Pin* createPin(IProps *props = nullptr) {
		return new Pin(props);
	}
};

class Module {
public:
	Module() : defaultPinFactory(new PinFactory), pinFactory(defaultPinFactory.get()) {}
	virtual ~Module() noexcept(false) {}
	virtual void process(std::shared_ptr<Data> data) = 0;
	Pin* getPin(size_t i) {
		return signals[i].get();
	}

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;
	std::unique_ptr<PinFactory> const defaultPinFactory;
	PinFactory* const pinFactory;
	std::vector<std::unique_ptr<Pin>> signals;
};

typedef IExecutor<void(std::shared_ptr<Data>)> IProcessExecutor;

inline size_t ConnectPin(Pin* p, std::function<void(std::shared_ptr<Data>)> functor, IProcessExecutor& executor) {
return p->getSignal().connect(functor, executor);
}

template<typename C, typename D, typename E>
size_t ConnectPin(Pin* p, C ObjectSlot, D MemberFunctionSlot, E& executor) {
auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
return ConnectPin(p, functor, executor);
}

}
