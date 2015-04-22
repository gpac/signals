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

	//Takes ownership
	template<typename T>
	T* addInput(T* p) {
		bool isDyn = false;
		std::unique_ptr<IInput> pEx(nullptr);
		if (inputs.size() && dynamic_cast<DataLoose*>(inputs.back().get())) {
			isDyn = true;
			pEx = std::move(inputs.back());
			inputs.pop_back();
		}
		inputs.push_back(uptr(p));
		if (isDyn)
			inputs.push_back(std::move(pEx));
		return p;
	}
	virtual size_t getNumInputs() const override {
		return maxRequestedInput + 1;
	}
	IInput* getInput(size_t i) override {
		if (i == inputs.size())
			addInput(new Input<DataLoose>(this));
		else if (i > inputs.size())
			throw std::runtime_error(format("Incorrect pin number %s for dynamic input.", i));

		maxRequestedInput = std::max(i+1, maxRequestedInput);
		return inputs[i].get();
	}

protected:
	size_t maxRequestedInput = 0; //FIXME: wrong by design. Asking for a pin doesn't need connecting to it (e.g. scanning for types when drawing a graph)
};
}
