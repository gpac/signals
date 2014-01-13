#pragma once


/* member function helper */

#define CONNECT(ObjectSig, MemberFunctionSig, ObjectSlot, MemberFunctionSlot) \
	(ObjectSig)->MemberFunctionSig.connect(MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot))

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
