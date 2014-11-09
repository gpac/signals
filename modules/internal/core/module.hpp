#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>


namespace Modules {

class Module {
public:
	Module() = default;

	virtual ~Module() noexcept(false) {
		for (auto &signal : pins) {
			signal->waitForCompletion();
		}
	}

	virtual void process(std::shared_ptr<Data> data) = 0;

	size_t getNumPin() const {
		return pins.size();
	}

	Pin* getPin(size_t i) {
		return pins[i].get();
	}

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;

	std::vector<std::unique_ptr<Pin>> pins;
};

}
