#pragma once

#include "../core/pin.hpp"
#include "../core/module.hpp"


namespace Modules {

/* member function helper */
#include "../../../signals/internal/utils/helper.hpp"

template<typename Class>
Signals::MemberFunctor<bool, Class, bool(Class::*)(std::shared_ptr<Data>)>
MEMBER_FUNCTOR_PROCESS(Class* objectPtr) {
	return Signals::MemberFunctor<bool, Class, bool(Class::*)(std::shared_ptr<Data>)>(objectPtr, &Class::process);
}

template<typename ModuleType>
size_t ConnectPinToModule(Pin* pin, ModuleType* module) {
	auto functor = MEMBER_FUNCTOR_PROCESS(module);
	return ConnectPin(pin, functor, module->getExecutor());
}

template<typename ModuleType>
size_t ConnectPinToModule(Pin* pin, std::unique_ptr<ModuleType>& module) {
	return ConnectPinToModule(pin, module.get());
}

template<typename SignalType, typename ModuleType>
size_t ConnectToModule(SignalType& sig, ModuleType* module) {
	auto functor = MEMBER_FUNCTOR_PROCESS(module);
	return Connect(sig, functor, module->getExecutor());
}

template<typename SignalType, typename ModuleType>
size_t ConnectToModule(SignalType& sig, std::unique_ptr<ModuleType>& module) {
	return ConnectToModule(sig, module.get());
}

}
