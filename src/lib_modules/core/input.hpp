#pragma once

#include "data.hpp"
#include "metadata.hpp"
#include "module.hpp"
#include "lib_signals/utils/queue.hpp"
#include <memory>


namespace Modules {

struct IInput : public ModuleS, public Metadata, public Signals::Queue<std::shared_ptr<const Data>> {
	virtual ~IInput() noexcept(false) {}
};

template<typename DataType>
class Input : public IInput {
public:
	Input(IModuleM * const module) : module(module) {}

	void process(std::shared_ptr<const Data> data) override {
		if (updateMetadata(data))
			module->flush();
		push(safe_cast<const DataType>(data));
		module->process();
	}

private:
	IModuleM * const module;
};

class InputCap {
public:
	virtual ~InputCap() noexcept(false) {}

	//Takes ownership/
	template<typename T>
	T* addInputPin(T* p) {
		inputs.push_back(uptr(p));
		return p;
	}
	size_t getNumInputPins() const {
		return inputs.size();
	}
	IInput* getInputPin(size_t i) const {
		return inputs[i].get();
	}

protected:
	std::vector<std::unique_ptr<IInput>> inputs;
};

}
