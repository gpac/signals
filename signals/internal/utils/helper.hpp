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
MEMBER_FUNCTOR(Class* objectPtr, Result (Class::*memberFunction) (Args...)) {
	return MemberFunctor<Result, Class, Result (Class::*)(Args...)>(objectPtr, memberFunction);
}

template<typename B, typename C, typename D, typename E>
void Connect(B& sig, C objectSlot, D memberFunctionSlot, E& executor) {
	auto functor = MEMBER_FUNCTOR(objectSlot, memberFunctionSlot);
	sig.connect(functor, executor);
}

template<typename B, typename C, typename D>
void Connect(B& sig, C objectSlot, D memberFunctionSlot) {
	Connect(sig, objectSlot, memberFunctionSlot, sig.getCaller());
}

template<typename SignalType, typename LambdaType, typename Executor>
void Connect(SignalType& sig, LambdaType lambda, Executor& executor) {
	sig.connect(lambda, executor);
}

template<typename SignalType, typename LambdaType>
void Connect(SignalType& sig, LambdaType lambda) {
	sig.connect(lambda);
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
