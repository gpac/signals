#pragma once


namespace Signals {

/* member function helper */

template<typename Result, typename Class, typename MemberFunction>
class MemberFunctor {
public:
	MemberFunctor(Class *object, MemberFunction function) : object(object), function(function) {
	}

	template<typename... Args>
	Result operator()(Args... args) {
		return (object->*function)(args...);
	}

private:
	Class *object;
	MemberFunction function;
};

template<typename Result, typename Class, typename... Args>
MemberFunctor<Result, Class, Result (Class::*)(Args...)>
MEMBER_FUNCTOR(Class* ObjectPtr, Result (Class::*MemberFunction) (Args...)) {
	return MemberFunctor<Result, Class, Result (Class::*)(Args...)>(ObjectPtr, MemberFunction);
}

template<typename B, typename C, typename D>
void Connect(B& Sig, C ObjectSlot, D MemberFunctionSlot)
{
	auto functor = MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot);
	Sig.connect(functor);
}

template<typename T>
struct NotVoidStruct {
	typedef T Type;
};

template<>
struct NotVoidStruct<void> {
	typedef int Type;
};

template <typename T> using NotVoid = typename NotVoidStruct<T>::Type;

}
