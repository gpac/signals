#pragma once

#include <../modules/modules.hpp>
#include "submodule.hpp"
#include <memory>

namespace MM {

//TODO: write an interface
//FIXME: this super module may look like a graph?
//FIXME: the preprocessor seems responsible for the data feed...
class MM_EXPORT Module {
public:
	static Module* create(Submodule *preprocessor, Modules::Module *module) {
		return new Module(preprocessor, module);
	}

	virtual ~Module() {
		for (size_t i = 0; i < signals.size(); ++i) {
			delete signals[i];
		}
	}

	bool process() {
		std::shared_ptr<Data> data(nullptr);
		return preprocessor->process(data);
	}

	bool reemit(std::shared_ptr<Data> data) {
		getSignal(0).emit(data);
		return true;
	}

	Pin::SignalType& getSignal(size_t i) {
		return signals[i]->getSignal();
	}

protected:
	std::vector<Pin*> signals;

private:
	/**
	* Take ownership of preprocessor and module
	*/
	Module(Submodule *preprocessor, Modules::Module *module) : preprocessor(preprocessor), module(module) {
		Connect(preprocessor->getPin(0)->getSignal(), module, &Modules::Module::process);
		for (size_t i = 0; i < module->getNumPin(); ++i) { //TODO: in reemit(), we must choose the right pin
			signals.push_back(new Pin); //module->getPin(i);
			Connect(module->getPin(i)->getSignal(), this, &MM::Module::reemit);
		}
	}
	std::unique_ptr<Submodule> preprocessor;
	std::unique_ptr<Modules::Module> module;
};

}
