#pragma once

#include "modules.hpp"

#include "../../utils/tools.hpp"


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

private:
	std::vector<std::unique_ptr<Module>> modules;
};

