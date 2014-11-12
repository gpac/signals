#pragma once

#include "internal/core/module.hpp"

namespace Modules {

namespace Utils {

class Recorder : public Module {
public:
	~Recorder();
	void process(std::shared_ptr<Data> data) override;
	void flush() override;

	std::shared_ptr<Data> pop();

private:
	Queue<std::shared_ptr<Data>> record;
};

}

}
