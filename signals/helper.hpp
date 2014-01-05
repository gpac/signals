#pragma once


/* member funtion helper */

#define MEMBER_FUNCTOR(Object, MemberFunction) MemberFunctor<decltype(get_return_type(&MemberFunction)), decltype(get_class(&MemberFunction)), decltype(&MemberFunction)>(&Object, &MemberFunction)

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
