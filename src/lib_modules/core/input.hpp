#pragma once

#include "data.hpp"
#include "metadata.hpp"
#include "lib_signals/utils/queue.hpp"
#include <atomic>
#include <memory>


namespace Modules {

struct IProcessor {
	virtual ~IProcessor() noexcept(false) {}
	virtual void process() = 0;
};

class ConnectedCap {
	public:
		ConnectedCap() : connections(0) {}
		virtual ~ConnectedCap() noexcept(false) {}
		virtual size_t getNumConnections() const {
			return connections;
		}
		virtual void connect() {
			connections++;
		}

	private:
		std::atomic_size_t connections;
};

struct IInput : public IProcessor, public ConnectedCap, public MetadataCap, public Signals::Queue<Data> {
	virtual ~IInput() noexcept(false) {}
};

template<typename DataType, typename ModuleType = IProcessor>
class Input : public IInput {
	public:
		Input(ModuleType * const module) : module(module) {}

		virtual void process() override {
			module->process();
		}

		virtual void push(Data data) override {
			if (typeid(DataType) == typeid(DataLoose))
				Signals::Queue<Data>::push(data);
			else
				Signals::Queue<Data>::push(safe_cast<const DataType>(data));
		}

	private:
		ModuleType * const module;
};

struct IInputCap {
	virtual ~IInputCap() noexcept(false) {}
	virtual size_t getNumInputs() const = 0;
	virtual IInput* getInput(size_t i) = 0;
	virtual IInput* addInput(IInput* p) = 0;
};

class InputCap : public virtual IInputCap {
	public:
		virtual ~InputCap() noexcept(false) {}

		//Takes ownership
		virtual IInput* addInput(IInput* p) override {
			inputs.push_back(uptr(p));
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
