#pragma once

#include "data.hpp"
#include "metadata.hpp"
#include "module.hpp"
#include "lib_signals/utils/queue.hpp"
#include <memory>


namespace Modules {

struct IInput : public IModule, public MetadataCap, public Signals::Queue<std::shared_ptr<const Data>> {
	virtual ~IInput() noexcept(false) {}
	virtual void process(std::shared_ptr<const Data> data) = 0;
	void flush() override {};
	void process() override {
		std::shared_ptr<const Data> data;
		if (tryPop(data))
			process(data);
	}
};

template<typename DataType>
class Input : public IInput {
public:
	Input(IModule * const module) : module(module) {}

	void process(std::shared_ptr<const Data> data) {
		if (updateMetadata(data))
			module->flush();
		push(safe_cast<const DataType>(data));
		module->process();
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
