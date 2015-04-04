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
size_t Connect(B& sig, C objectSlot, D memberFunctionSlot, E& executor) {
	auto functor = MEMBER_FUNCTOR(objectSlot, memberFunctionSlot);
	return sig.connect(functor, executor);
}

template<typename B, typename C, typename D>
size_t Connect(B& sig, C objectSlot, D memberFunctionSlot) {
	return Connect(sig, objectSlot, memberFunctionSlot, sig.getExecutor());
}

template<typename SignalType, typename LambdaType, typename Executor>
size_t Connect(SignalType& sig, LambdaType lambda, Executor& executor) {
	return sig.connect(lambda, executor);
}

template<typename SignalType, typename LambdaType>
size_t Connect(SignalType& sig, LambdaType lambda) {
	return sig.connect(lambda);
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
