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
	virtual void process(Data data) override { //Romain: to remove once pipeline move to 'Module' instead of 'ModuleS'
		if (inputs.size() == 0)
			addInput(new Input<DataBase>(this));
		assert(inputs.size() == 1);
		inputs[0]->process(data);
	}
	void process(bool /*dataTypeUpdated*/) override {
		assert(0);
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

private:
	ModuleS(ModuleS const&) = delete;
	ModuleS const& operator=(ModuleS const&) = delete;
};
}
