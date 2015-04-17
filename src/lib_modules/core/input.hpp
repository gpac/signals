#pragma once

#include "data.hpp"
#include "metadata.hpp"
#include <memory>


namespace Modules {

class IModule;

class IProcessor { //FIXME: template + there should be no module with no input from now, so module doesn't need to be a public processor
public:
	virtual ~IProcessor() noexcept(false) {};
	virtual void process(std::shared_ptr<const Data> data) = 0;
	virtual void flush() {};
};

struct IInput : public IProcessor, public Metadata {
	virtual ~IInput() noexcept(false) {}
};

template<typename DataType>
class Input : public IInput {
public:
	Input(IProcessor * const module) : module(module) {}

	void process(std::shared_ptr<const Data> data) {
		if (updateMetadata(data))
			module->flush();
		module->process(safe_cast<const DataType>(data));
	}

private:
	IProcessor * const module;
};

}
