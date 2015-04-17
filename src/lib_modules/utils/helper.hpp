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

template<typename SignalType, typename ModuleType>
size_t ConnectToModule(SignalType& sig, ModuleType* module, IProcessExecutor& executor) {
	auto functor = MEMBER_FUNCTOR_PROCESS(module);
	return Connect(sig, functor, executor);
}

template<typename SignalType, typename ModuleType>
size_t ConnectToModule(SignalType& sig, std::unique_ptr<ModuleType>& module, IProcessExecutor& executor = defaultExecutor) {
	return ConnectToModule(sig, module.get(), executor);
}

template <typename T>
std::shared_ptr<const T> getMetadataFromOutput(IOutput const * const out) {
	auto const metadata = safe_cast<const IMetadataCap>(out)->getMetadata();
	return safe_cast<const T>(metadata);
}

}
