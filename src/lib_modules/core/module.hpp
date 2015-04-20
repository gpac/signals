#pragma once

#include "data.hpp"
#include "input.hpp"
#include "output.hpp"
#include <memory>
#include <vector>


namespace Modules {

struct IModule {
	virtual ~IModule() noexcept(false) {}
	virtual void process() = 0;
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
	virtual void process() override {
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
			throw std::runtime_error(format("Incorrect pin number %s for dynamic input.", i));

		return inputs[i].get();
	}
};
}
