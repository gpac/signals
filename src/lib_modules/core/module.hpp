#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>


namespace Modules {

class Input;

class IModule {
public:
	virtual ~IModule() noexcept(false) {}
	virtual void process(std::shared_ptr<const Data> data) = 0;
	virtual size_t getNumInputPins() const { return 0; } //TODO: choose to make it mandatory or transparent
	virtual Input* getInputPin(size_t i) { return nullptr; } //TODO: choose to make it mandatory or transparent
	virtual size_t getNumOutputPins() const = 0;
	virtual IPin* getOutputPin(size_t i) const = 0;
};

class Input {
public:
	Input(IModule * const module) : module(module) {}

	void process(std::shared_ptr<const Data> data) {
		//TODO: check media type
		module->process(data);
	}
	//getMediaType() //TODO

private:
	IModule * const module;
};

class Module : public IModule {
public:
	Module() = default;

	virtual ~Module() noexcept(false) {}
	virtual void process(std::shared_ptr<const Data> data) = 0;
	virtual void flush() {};

	size_t getNumInputPins() const override {
		return inputPins.size();
	}
	Input* getInputPin(size_t i) override {
		if (i < getNumInputPins()) {
			return inputPins[i].get();
		} else if (i == getNumInputPins()) {
			Log::msg(Log::Debug, "[Module] Implicit input [%s] creation", i);
			auto p = uptr(new Input(this));
			inputPins.push_back(std::move(p));
			return inputPins[i].get();
		} else {
			throw std::runtime_error("[Module] Input is not in range. Failed.");
		}
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

	template<typename T>
	T* addOutputPin(T* p) {
		if (m_isLowLatency)
			p->setAllocator(new typename T::AllocatorType(ALLOC_NUM_BLOCKS_LOW_LATENCY));
		outputPins.push_back(uptr(p));
		return p;
	}

private:
	std::vector<std::unique_ptr<Input>> inputPins; //TODO: don't name them 'pin'
	std::vector<std::unique_ptr<IPin>> outputPins; //TODO: don't name them 'pin'
	bool m_isLowLatency = false;
};

}
