#pragma once

#include "data.hpp"
#include "output.hpp"
#include <memory>
#include <vector>


namespace Modules {
struct IModule {
	virtual ~IModule() noexcept(false) {}
	virtual void process(bool dataTypeUpdated) = 0;
	virtual void flush() = 0;
};
}

#include "input.hpp"

namespace Modules {
class Module : public IModule, public InputCap, public OutputCap {
public:
	Module() = default;
	virtual ~Module() noexcept(false) {}

private:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;
};

//single input specialized module
class ModuleS : public IInput, public IInputCap, public OutputCap {
public:
	ModuleS() = default;
	virtual ~ModuleS() noexcept(false) {}
	virtual void process(Data data) = 0;
	size_t getNumInputs() const override {
		return 1;
	}
	IInput* getInput(size_t i) override {
		return this;
	}

private:
	ModuleS(ModuleS const&) = delete;
	ModuleS const& operator=(ModuleS const&) = delete;
};
}
