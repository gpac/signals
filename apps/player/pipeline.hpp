#pragma once

#include "modules.hpp"

class Pipeline {
public:
	void add(Module* module) {
		modules.push_back(uptr(module));
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
	std::vector<std::unique_ptr<Module>> modules;
};

