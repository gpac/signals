#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>


namespace Modules {

class IProcessor { //FIXME: template + there should be no module with no pin from now, so module doesn't need to be a public processor
public:
	virtual ~IProcessor() noexcept(false) {};
	virtual void process(std::shared_ptr<const Data> data) = 0;
};

class IModule : public IProcessor {
public:
	virtual ~IModule() noexcept(false) {}
	virtual void process(std::shared_ptr<const Data> data) override = 0;
	virtual void flush() {};
	virtual size_t getNumOutputPins() const = 0;
	virtual IPin* getOutputPin(size_t i) const = 0;
};

template<typename DataType>
class Input : public IProcessor, protected PropsHandler {
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
		inputPins.push_back(uptr(p));
		return p;
	}
	size_t getNumInputPins() const {
		return inputPins.size();
	}
	IProcessor* getInputPin(size_t i) const {
		return inputPins[i].get();
	}

	size_t getNumOutputPins() const override {
		return outputPins.size();
	}
	IPin* getOutputPin(size_t i) const override {
		return outputPins[i].get();
	}

	void setLowLatency(bool isLowLatency) {
		m_isLowLatency = isLowLatency;
	}

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;

	//Takes ownership
	template<typename T>
	T* addOutputPin(T* p) {
		if (m_isLowLatency)
			p->setAllocator(new typename T::AllocatorType(ALLOC_NUM_BLOCKS_LOW_LATENCY));
		outputPins.push_back(uptr(p));
		return p;
	}

private:
	std::vector<std::unique_ptr<IProcessor>> inputPins; //TODO: don't name them 'pin'
	std::vector<std::unique_ptr<IPin>> outputPins; //TODO: don't name them 'pin'
	bool m_isLowLatency = false;
};

}
