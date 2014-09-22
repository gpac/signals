#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>
#include <string>


namespace Modules {

class IModule {
public:
	virtual ~IModule() noexcept(false) {};

	virtual bool process(std::shared_ptr<Data> data) = 0;
	virtual void waitForCompletion() = 0; /* required for async, otherwise we still have callback/futures on an object being destroyed */
};

class Module : public IModule {
public:
	Module(PinFactory *pinFactory) : pinFactory(pinFactory), defaultExecutor(new ExecutorSync<bool(std::shared_ptr<Data>)>()), executor(*defaultExecutor.get()) {
	}
	Module() : defaultPinFactory(new PinDefaultFactory), pinFactory(defaultPinFactory.get()), defaultExecutor(new ExecutorSync<bool(std::shared_ptr<Data>)>()), executor(*defaultExecutor.get()) {
	}

	std::function<bool(std::shared_ptr<Data>)> getInput() {
		auto processFunction = [&](std::shared_ptr<Data> data) -> bool {
			return process(data);
		};
		return processFunction;
	}

	/**
	 * Must be called before the destructor.
	 */
	virtual void waitForCompletion() {
		for (auto &signal : signals) {
			signal->waitForCompletion();
		}
	}

	size_t getNumPin() const {
		return signals.size();
	}

	Pin* getPin(size_t i) {
		return signals[i].get();
	}

	IExecutor<bool(std::shared_ptr<Data>)>& getExecutor() const {
		return executor;
	}

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;

	std::unique_ptr<PinFactory> const defaultPinFactory;
	PinFactory* const pinFactory;
	std::vector<std::unique_ptr<Pin>> signals;

	std::unique_ptr<IExecutor<bool(std::shared_ptr<Data>)>> const defaultExecutor;
	IExecutor<bool(std::shared_ptr<Data>)> &executor;
};

}
