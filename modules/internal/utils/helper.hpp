#pragma once

#include "stranded_pool_executor.hpp"
#include "../../../signals/internal/utils/helper.hpp" /* member function helper */
#include <memory>


namespace Modules {

class Pin;
class Data;

template<typename Class>
Signals::MemberFunctor<void, Class, void(Class::*)(std::shared_ptr<Data>)>
MEMBER_FUNCTOR_PROCESS(Class* objectPtr) {
	return Signals::MemberFunctor<void, Class, void(Class::*)(std::shared_ptr<Data>)>(objectPtr, &Class::process);
}

template<typename ModuleType>
size_t ConnectPinToModule(Pin* pin, ModuleType* module, IProcessExecutor& executor) {
	auto functor = MEMBER_FUNCTOR_PROCESS(module);
	return ConnectPin(pin, functor, executor);
}

template<typename ModuleType>
size_t ConnectPinToModule(Pin* pin, std::unique_ptr<ModuleType>& module, IProcessExecutor& executor = defaultExecutor) {
	return ConnectPinToModule(pin, module.get(), executor);
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

}
