#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>
#include <string>


namespace Modules {

class MODULES_EXPORT IModule {
public:
	virtual bool process(std::shared_ptr<Data> data) = 0;
	virtual bool handles(const std::string &url) = 0;
	virtual void destroy() = 0; /* required for async, otherwise we still have callback/futures on an object being destroyed */
};

template<typename PinType>
class ModuleT : public IModule {
public:
	virtual ~ModuleT() {
	}

	virtual bool process(std::shared_ptr<Data> data) = 0;
	virtual bool handles(const std::string &url) = 0;

	/**
	 * Must be called before the destructor.
	 */
	virtual void destroy() {
		for (auto &signal : signals) {
			signal->destroy();
		}
	}

	Pin::SignalType& getSignal(size_t i) {
		return signals[i]->getSignal();
	}

	size_t getNumPin() const {
		return signals.size();
	}

	const Pin* getPin(size_t i) {
		return signals[i];
	}

protected:
	ModuleT() = default;
	ModuleT(ModuleT const&) = delete;
	ModuleT const& operator=(ModuleT const&) = delete;

	std::vector<PinType*> signals;
};

typedef MODULES_EXPORT ModuleT<Pin> Module;
typedef MODULES_EXPORT ModuleT<PinSync> ModuleSync;

}
