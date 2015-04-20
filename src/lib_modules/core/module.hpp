#pragma once

#include "data.hpp"
#include "input.hpp"
#include "output.hpp"
#include <memory>
#include <vector>


namespace Modules {

struct IModule {
	virtual ~IModule() noexcept(false) {}
	virtual void process(bool dataTypeUpdated) = 0;
	virtual void flush() = 0;
};

class Module : public IModule, public InputCap, public OutputCap {
public:
	Module() = default;
	virtual ~Module() noexcept(false) {}
	virtual void flush() {}

private:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;
};

//single input specialized module
class ModuleS : public Module {
public:
	ModuleS() = default;
	virtual ~ModuleS() noexcept(false) {}
	virtual void process(Data data) = 0;
	virtual void process(bool /*dataTypeUpdated*/) override {
		process(getInput(0)->pop());
	}
};

//dynamic input number specialized module
class ModuleDynI : public Module {
public:
	ModuleDynI() = default;
	virtual ~ModuleDynI() noexcept(false) {}

	virtual size_t getNumInputs() const override {
		return inputs.size() + 1;
	}
	IInput* getInput(size_t i) override {
		if (i == inputs.size())
			addInput(new Input<DataLoose>(this));
		else if (i > inputs.size())
			throw std::runtime_error("Incorrect pin number for dynamic input.");

		return inputs[i].get();
	}
};
}
