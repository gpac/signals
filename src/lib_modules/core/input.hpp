#pragma once

#include "data.hpp"
#include "metadata.hpp"
#include "module.hpp"
#include "lib_signals/utils/queue.hpp"
#include <memory>


namespace Modules {

struct IInput : public IModule, public MetadataCap, public Signals::Queue<Data> {
	virtual ~IInput() noexcept(false) {}
	virtual void process(Data data) = 0;
	virtual void flush() override {};
	virtual void process(bool /*dataTypeUpdated*/) override {
		Data data;
		if (tryPop(data))
			process(data);
	}
};

template<typename DataType>
class Input : public IInput {
public:
	Input(IModule * const module) : module(module) {}

	void process(Data data) override {
		push(safe_cast<const DataType>(data));
		module->process(updateMetadata(data));
	}

private:
	IModule * const module;
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
