#pragma once

#include "stranded_pool_executor.hpp"
#include "../core/module.hpp"
#include "lib_signals/utils/helper.hpp"
#include <memory>


namespace Modules {

template<typename Class>
Signals::MemberFunctor<void, Class, void(Class::*)(Data)>
MEMBER_FUNCTOR_PROCESS(Class* objectPtr) {
	return Signals::MemberFunctor<void, Class, void(Class::*)(Data)>(objectPtr, &IProcessor::process);
}

inline size_t ConnectOutput(IOutput* p, std::function<void(Data)> functor) {
	return p->getSignal().connect(functor);
}

template<typename C, typename D>
size_t ConnectOutput(IOutput* p, C ObjectSlot, D MemberFunctionSlot) {
	auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
	return ConnectOutput(p, functor);
}

template<typename ModuleType>
size_t ConnectOutputToInput(IOutput* output, ModuleType* module, IProcessExecutor& executor = defaultExecutor) {
	if (safe_cast<const IMetadataCap>(output)->getMetadata() && module->getMetadata()) {
		if (safe_cast<const IMetadataCap>(output)->getMetadata()->getStreamType() != module->getMetadata()->getStreamType())
			throw std::runtime_error("Module connection: incompatible types");
		Log::msg(Log::Info, "--------- OK");
	} else {
		Log::msg(Log::Info, "--------- output meta: %s, input meta: %s", safe_cast<const IMetadataCap>(output)->getMetadata(), module->getMetadata());
	}
	auto functor = MEMBER_FUNCTOR_PROCESS(module);
	return output->getSignal().connect(functor, executor);
}

template<typename ModuleType>
size_t ConnectOutputToInput(IOutput* output, std::unique_ptr<ModuleType>& module, IProcessExecutor& executor = defaultExecutor) {
	return ConnectOutputToInput(output, module.get(), executor);
}

template<typename OutputType, typename ModuleType>
size_t ConnectModules(OutputType *output, ModuleType *module, size_t inputIdx, IProcessExecutor& executor = defaultExecutor) {
	auto input = module->getInput(inputIdx);
	return ConnectOutputToInput(output, input, executor);
}

template<typename ModuleType1, typename ModuleType2>
size_t ConnectModules(ModuleType1 *module1, size_t outputIdx, ModuleType2 *module2, size_t inputIdx, IProcessExecutor& executor = defaultExecutor) {
	auto output = module1->getOutput(outputIdx);
	return ConnectModules(output, module2, inputIdx, executor);
}

template <typename T>
std::shared_ptr<const T> getMetadataFromOutput(IOutput const * const out) {
	auto const metadata = safe_cast<const IMetadataCap>(out)->getMetadata();
	return safe_cast<const T>(metadata);
}

}
