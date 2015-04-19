#pragma once

#include "data.hpp"
#include "metadata.hpp"
#include "lib_signals/utils/queue.hpp"
#include <memory>


namespace Modules {

struct IInput : public MetadataCap, public Signals::Queue<Data> {
	virtual ~IInput() noexcept(false) {}
	virtual void process(Data data) = 0;
};

template<typename DataType>
class Input : public IInput {
public:
	Input(IModule * const module) : module(module) {} //Romain: remove IModule

	void process(Data data) override {
		push(safe_cast<const DataType>(data));
		module->process(updateMetadata(data));
	}

private:
	IModule * const module;
};

struct IInputCap {
	virtual ~IInputCap() noexcept(false) {}
	virtual size_t getNumInputs() const = 0;
	virtual IInput* getInput(size_t i) = 0;
};

class InputCap : public IInputCap {
public:
	virtual ~InputCap() noexcept(false) {}

	//Takes ownership/
	template<typename T>
	T* addInput(T* p) {
		inputs.push_back(uptr(p));
		return p;
	}
	size_t getNumInputs() const override {
		return inputs.size();
	}
	IInput* getInput(size_t i) override {
		return inputs[i].get();
	}

protected:
	std::vector<std::unique_ptr<IInput>> inputs;
};

}
