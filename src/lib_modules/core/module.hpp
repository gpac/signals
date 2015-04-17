#pragma once

#include "data.hpp"
#include <memory>
#include <vector>


namespace Modules {
struct IModule {
	virtual ~IModule() noexcept(false) {}
	virtual void flush() = 0;
};
}

//single input module
#include "output.hpp"
namespace Modules {
class ModuleS : public IModule, public OutputCap {
public:
	ModuleS() = default;
	virtual ~ModuleS() noexcept(false) {}
	virtual void process(std::shared_ptr<const Data> data) = 0;
	virtual void flush() {};

private:
	ModuleS(ModuleS const&) = delete;
	ModuleS const& operator=(ModuleS const&) = delete;
};
}

//multiple input module
namespace Modules {
struct IModuleM : public IModule {
	virtual ~IModuleM() {}
	virtual void process() = 0;
};
}
#include "input.hpp"
namespace Modules {
class ModuleM : public IModuleM, public InputCap, public OutputCap {
public:
	ModuleM() = default;
	virtual void process() {
		if (inputs.size() == 1) {
			std::shared_ptr<const Data> data;
			//if (inputs[0]->tryPop(data))
			//	process(data);
		}
	}

private:
	ModuleM(ModuleM const&) = delete;
	ModuleM const& operator=(ModuleM const&) = delete;
};
}
