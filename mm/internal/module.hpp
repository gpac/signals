#pragma once

#include <../modules/modules.hpp>
#include "submodule.hpp"

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
		delete preprocessor;
		delete module;
		for (size_t i = 0; i < signals.size(); ++i) {
			delete signals[i];
		}
	}

	bool process() {
		std::shared_ptr<Data> data(nullptr);
		return preprocessor->process(data);
	}

	bool reemit(std::shared_ptr<Data> data) {
		signals[0]->signal.emit(data);
		return true;
	}

	std::vector<Pin<>*> signals;

private:
	/**
	* Take ownership of preprocessor and module
	*/
	Module(Submodule *preprocessor, Modules::Module *module) : preprocessor(preprocessor), module(module) {
		CONNECT(preprocessor, signals[0]->signal, module, &Modules::Module::process);
		assert(module->signals.size() == 1); //TODO: in reemit(), we must choose the right pin
		signals.resize(module->signals.size());
		for (size_t i = 0; i < module->signals.size(); ++i) {
			signals[i] = new Pin<>;// module->signals[i];
			CONNECT(module, signals[i]->signal, this, &MM::Module::reemit);
		}
	}
	Submodule *preprocessor;
	Modules::Module *module;
};

}
