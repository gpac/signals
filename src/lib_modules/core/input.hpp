#pragma once

#include "data.hpp"
#include "metadata.hpp"
#include "lib_signals/utils/queue.hpp"
#include <atomic>
#include <memory>


namespace Modules {

struct IProcessor {
	virtual ~IProcessor() noexcept(false) {}
	virtual void process(Data data) = 0;
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

struct IModule;

template<typename DataType, typename ModuleType = IModule>
class Input : public IInput {
	public:
		Input(ModuleType * const module) : module(module) {}

		virtual void process(Data data) override {
			if (typeid(DataType) == typeid(DataLoose))
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
