#pragma once

#include "data.hpp"
#include "output.hpp"
#include <memory>
#include <vector>


namespace Modules {
struct IModule {
	virtual ~IModule() noexcept(false) {}
	virtual void process() = 0;
	virtual void flush() = 0;
};
}

#include "input.hpp"

namespace Modules {
class Module : public IModule, public InputCap, public OutputCap {
public:
	Module() = default;
	void process() override;

private:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;
};

//single input specialized module
class ModuleS : public IInput, public OutputCap {
public:
	ModuleS() = default;
	virtual ~ModuleS() noexcept(false) {}

private:
	ModuleS(ModuleS const&) = delete;
	ModuleS const& operator=(ModuleS const&) = delete;
};
}
