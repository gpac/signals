#pragma once


/* member funtion helper */

#define MEMBER_FUNCTOR(ObjectPtr, MemberFunctionPtr) MemberFunctor<decltype(get_return_type(MemberFunctionPtr)), decltype(get_class(MemberFunctionPtr)), decltype(MemberFunctionPtr)>(ObjectPtr, MemberFunctionPtr)

#define CONNECT(ObjectSig, MemberFunctionSig, ObjectSlot, MemberFunctionSlot) (ObjectSig)->MemberFunctionSig.connect(MEMBER_FUNCTOR(ObjectSlot, MemberFunctionSlot))

template<typename Result, typename Class, typename... Args>
Result get_return_type(Result(Class::*)(Args...));

template<typename Result, typename Class, typename... Args>
Class get_class(Result(Class::*)(Args...));

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
