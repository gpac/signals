#pragma once

#include "data.hpp"
#include "metadata.hpp"
#include "lib_signals/utils/queue.hpp"
#include <memory>


namespace Modules {

struct IProcessor {
	virtual ~IProcessor() noexcept(false) {}
	virtual void process(Data data) = 0;
};

struct IInput : public IProcessor, public MetadataCap, public Signals::Queue<Data> {
	virtual ~IInput() noexcept(false) {}
};

struct IModule;

template<typename DataType, typename ModuleType = IModule>
class Input : public IInput {
public:
	Input(ModuleType * const module) : module(module) {}

	virtual void process(Data data) override {
		if (typeid(DataType) == typeid(DataLoose)) //FIXME
			push(data);
		else
			push(safe_cast<const DataType>(data));
		module->process();
	}

private:
	ModuleType * const module;
};

struct IInputCap {
	virtual ~IInputCap() noexcept(false) {}
	virtual size_t getNumInputs() const = 0;
	virtual IInput* getInput(size_t i) = 0;
};

class InputCap : public IInputCap {
public:
	virtual ~InputCap() noexcept(false) {}

	//Takes ownership
	template<typename T>
	T* addInput(T* p) {
		//Romain: ultra-bourrin: en vrai on ne veut pas changer une pin déjà connectée
		bool isDyn = false;
		std::unique_ptr<IInput> pEx(nullptr);
		if (inputs.size() && dynamic_cast<DataLoose*>(inputs.back().get())) {
			isDyn = true;
			pEx = std::move(inputs.back());
			inputs.pop_back();
		}
		inputs.push_back(uptr(p));
		if (isDyn)
			inputs.push_back(std::move(pEx));
		return p;
	}
	virtual size_t getNumInputs() const override {
		return inputs.size();
	}
	virtual IInput* getInput(size_t i) override {
		return inputs[i].get();
	}

protected:
	std::vector<std::unique_ptr<IInput>> inputs;
};

}
