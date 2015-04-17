#pragma once

#include "data.hpp"
#include "output.hpp"
#include <vector>


namespace Modules {

class IProcessor { //FIXME: template + there should be no module with no input from now, so module doesn't need to be a public processor
public:
	virtual ~IProcessor() noexcept(false) {};
	virtual void process(std::shared_ptr<const Data> data) = 0;
};

class IModule : public IProcessor {
public:
	virtual ~IModule() noexcept(false) {}
	virtual void flush() {};
	virtual size_t getNumOutputs() const = 0;
	virtual IOutput* getOutput(size_t i) const = 0;
};

struct IInput : public IProcessor, public Metadata {
	virtual ~IInput() noexcept(false) {}
};

template<typename DataType>
class Input : public IInput {
public:
	Input(IModule * const module) : module(module) {}

	void process(std::shared_ptr<const Data> data) {
		if (updateMetadata(data))
			module->flush();
		module->process(safe_cast<const DataType>(data));
	}

private:
	IModule * const module;
};

class Module : public IModule {
public:
	Module() = default;

	virtual ~Module() noexcept(false) {}
	virtual void process(std::shared_ptr<const Data> data) = 0;

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

	size_t getNumOutputs() const override {
		return outputs.size();
	}
	IOutput* getOutput(size_t i) const override {
		return outputs[i].get();
	}

	void setLowLatency(bool isLowLatency) {
		m_isLowLatency = isLowLatency;
	}

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;

	//Takes ownership
	template<typename T>
	T* addOutput(T* p) {
		if (m_isLowLatency)
			p->setAllocator(new typename T::AllocatorType(ALLOC_NUM_BLOCKS_LOW_LATENCY));
		outputs.push_back(uptr(p));
		return p;
	}

private:
	std::vector<std::unique_ptr<IInput>> inputs;
	std::vector<std::unique_ptr<IOutput>> outputs;
	bool m_isLowLatency = false;
};

}
