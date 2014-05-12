#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>
#include <string>


namespace Modules {

class IModule {
public:
	virtual ~IModule() {};

	virtual bool process(std::shared_ptr<Data> data) = 0;
	virtual bool handles(const std::string &url) = 0;
	virtual void waitForCompletion() = 0; /* required for async, otherwise we still have callback/futures on an object being destroyed */
};

class Module : public IModule {
public:
	Module(PinFactory *pinFactory) : pinFactory(pinFactory) {
	}
	Module() : defaultPinFactory(new PinDefaultFactory), pinFactory(defaultPinFactory.get()) {
	}

	virtual bool handles(const std::string &url) = 0;

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

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;

	std::unique_ptr<PinFactory> const defaultPinFactory;
	PinFactory* const pinFactory;
	std::vector<std::unique_ptr<Pin>> signals;
};

}
