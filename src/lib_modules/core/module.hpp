#pragma once

#include "data.hpp"
#include "error.hpp"
#include "input.hpp"
#include "log.hpp"
#include "output.hpp"
#include <memory>
#include <string>
#include <vector>


namespace Modules {

struct IModule {
	virtual ~IModule() noexcept(false) {}
	virtual void process() = 0;
	virtual void flush() = 0;
};

class Module : public IModule, public IError, public LogCap, public InputCap, public OutputCap {
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
//note: pins added automatically will carry the DataLoose type which doesn't
//      allow to perform all safety checks ; consider adding pins manually if
//      you can
class ModuleDynI : public Module {
	public:
		ModuleDynI() = default;
		virtual ~ModuleDynI() noexcept(false) {}

		//takes ownership
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
			if (inputs.size() == 0)
				return 1;
			else if (inputs[inputs.size() - 1]->getNumConnections() == 0)
				return inputs.size();
			else
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
