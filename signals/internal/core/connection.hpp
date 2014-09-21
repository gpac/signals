#pragma once

#include <cassert>
#include <future>
#include <vector>

namespace Signals {

template<typename ResultType, typename... Args>
class ConnectionList { //TODO: write interface
public:
	typedef NotVoid<ResultType> FutureResultType;
	IExecutor<ResultType(Args...)> &executor;
	std::function<ResultType(Args...)> const callback;
	size_t const uid;
	std::vector<std::shared_future<FutureResultType>> futures;

	explicit ConnectionList(IExecutor<ResultType(Args...)> &executor, const std::function<ResultType(Args...)> &callback, const size_t uid) : executor(executor), callback(callback), uid(uid) {
	}
};

}
