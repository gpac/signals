#pragma once

#include "stranded_pool_executor.hpp"
#include "lib_signals/utils/helper.hpp"
#include <memory>


namespace Modules {

struct IOutput;

template<typename Class>
Signals::MemberFunctor<void, Class, void(Class::*)(Data)>
MEMBER_FUNCTOR_PROCESS(Class* objectPtr) {
	return Signals::MemberFunctor<void, Class, void(Class::*)(Data)>(objectPtr, &Class::process);
}

template<typename ModuleType>
size_t ConnectOutputToModule(IOutput* out, ModuleType* module, IProcessExecutor& executor = defaultExecutor) {
	auto functor = MEMBER_FUNCTOR_PROCESS(module);
	return ConnectOutput(out, functor, executor);
}

template<typename ModuleType>
size_t ConnectOutputToModule(IOutput* out, std::unique_ptr<ModuleType>& module, IProcessExecutor& executor = defaultExecutor) {
	return ConnectOutputToModule(out, module.get(), executor);
}

template<typename OutputType, typename ModuleType>
size_t ConnectModules(OutputType *output, ModuleType *module, size_t inputIdx, IProcessExecutor& executor = defaultExecutor) {
	auto input = module->getInput(inputIdx);
	auto functor = MEMBER_FUNCTOR_PROCESS(input);
	//TODO: check types:http://www.cplusplus.com/reference/typeinfo/type_info/
	return ConnectOutput(output, functor, executor);
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
