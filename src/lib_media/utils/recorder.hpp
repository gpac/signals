#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {

namespace Utils {

class Recorder : public ModuleS {
public:
	Recorder();
	void process(Data data) override;
	void flush() override;

	Data pop();

private:
	Queue<Data> record;
};

}

}
