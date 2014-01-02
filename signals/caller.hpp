#pragma once

#include <functional>


template<typename, typename> class CallerSync;

template<typename Result, typename Callback, typename... Args>
class CallerSync<Result, Callback(Args...)> {
public:
	static void call(Result &result, const std::function<Callback(Args...)> &callback, Args... args) {
		result(callback(args...));
	}
};
