#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {

namespace Utils {

class Recorder : public ModuleS {
public:
	void process(std::shared_ptr<const Data> data) override;
	void flush() override;

	std::shared_ptr<const Data> pop();

private:
	Queue<std::shared_ptr<const Data>> record;
};

}

}
