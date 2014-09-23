#pragma once

#include "../core/pin.hpp"
#include "../core/module.hpp"


namespace Modules {

/* Safe Module helper */

template<typename> class ModuleSafe;

template<typename T>
class ModuleSafe {
public:
	ModuleSafe(T *module) : module(module) {
	}

	virtual ~ModuleSafe() {
		if (module)
			module->waitForCompletion();
	}

	ModuleSafe(ModuleSafe&& other) = default;

	template<typename M>
	ModuleSafe(ModuleSafe<M>&& other) : module(std::move(other.module)) {
	}

	T* operator->() const {
		return module.operator->();
	}
	
	T* get() const {
		return module.operator->();
	}

	bool operator!() const {
		return module.get() != nullptr;
	}

#ifndef _WIN32
private: //FIXME: MSVC2014.3 CTP1 bug?
#endif
	std::unique_ptr<T> module;

private:
	ModuleSafe(ModuleSafe const&) = delete;
	ModuleSafe const& operator=(ModuleSafe const&) = delete;
};

#ifdef UPTR_DEFINED
# error Do not define 'uptr' before including this helper which makes additional safety checks.
#else
template<typename T>
std::unique_ptr<T> uptr(T *p) {
	static_assert(!std::is_base_of<Module, T>::value, "Unsafe uptr used with Modules::Module. Use uptrSafeModule instead.");
	return std::unique_ptr<T>(p);
}
#endif

template<typename T>
ModuleSafe<T> uptrSafeModule(T *p) {
	static_assert(std::is_base_of<Module, T>::value, "Use uptrSafeModule with Modules::Module only.");
	return ModuleSafe<T>(p);
}


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

template<typename ModuleType>
size_t ConnectPinToModule(Pin* pin, ModuleSafe<ModuleType>& module) {
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

template<typename SignalType, typename ModuleType>
size_t ConnectToModule(SignalType& sig, ModuleSafe<ModuleType>& module) {
	return ConnectToModule(sig, module.get());
}

}
