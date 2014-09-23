#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>
#include <string>


namespace Modules {

class Module {
public:
	Module(PinFactory *pinFactory) : pinFactory(pinFactory), defaultExecutor(new ExecutorSync<bool(std::shared_ptr<Data>)>()), executor(*defaultExecutor.get()) {
	}
	Module() : defaultPinFactory(new PinDefaultFactory), pinFactory(defaultPinFactory.get()), defaultExecutor(new ExecutorSync<bool(std::shared_ptr<Data>)>()), executor(*defaultExecutor.get()) {
	}
	virtual ~Module() {
		for (auto &signal : signals) {
			signal->waitForCompletion();
		}
	}

	virtual bool process(std::shared_ptr<Data> data) = 0;

	size_t getNumPin() const {
		return signals.size();
	}

	Pin* getPin(size_t i) {
		return signals[i].get();
	}

	IProcessExecutor& getExecutor() const {
		return executor;
	}

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;

	std::unique_ptr<PinFactory> const defaultPinFactory;
	PinFactory* const pinFactory;
	std::vector<std::unique_ptr<Pin>> signals;

	std::unique_ptr<IProcessExecutor> const defaultExecutor;
	IProcessExecutor &executor;
};

}
