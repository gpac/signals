#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>
#include <string>


namespace Modules {

class Module {
public:
	Module(PinFactory *pinFactory) : pinFactory(pinFactory) {
	}
	Module() : defaultPinFactory(new PinDefaultFactory), pinFactory(defaultPinFactory.get()) {
	}
	virtual ~Module() noexcept(false) {
		for (auto &signal : signals) {
			signal->waitForCompletion();
		}
	}

	virtual void process(std::shared_ptr<Data> data) = 0;

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
