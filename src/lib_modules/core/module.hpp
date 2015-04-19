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
	Module()// = default; //Romain: change on real multiple-output: muxers could create their automatically is dynamic (Cap?)
	{
		addInput(new Input<DataBase>(this));
	}
	virtual ~Module() noexcept(false) {}
	virtual void flush() {}
	virtual void process(Data data) { //Romain: to remove once pipeline move to 'Module' instead of 'ModuleS'
		if (inputs.size() == 0)
			addInput(new Input<DataBase>(this));
		assert(inputs.size() == 1);
		inputs[0]->process(data);
	}

private:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;
};

//single input specialized module
class ModuleS : public Module {
public:
	ModuleS() {
		addInput(new Input<DataBase>(this));
	}
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

	IInput* getInput(size_t i) override {
		if (i == inputs.size())
			addInput(new Input<DataBase>(this));
		else if (i > inputs.size())
			throw std::runtime_error("Incorrect pin number for dynamic input.");

		return inputs[i].get();
	}
};
}
