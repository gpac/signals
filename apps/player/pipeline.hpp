#pragma once

#include "modules.hpp"

#include "../../utils/tools.hpp"


class Pipeline {
public:
	void add(Module* module) {
		modules.push_back(uptrSafeModule(module));
	}

	void run() {
		auto& sourceModule = modules[0];
		while (sourceModule->process(nullptr)) {
		}
	}

	~Pipeline() {
		foreach(i, modules) {
			(*i)->waitForCompletion();
		}
	}

private:
	std::vector<ModuleSafe<Module>> modules;
};

