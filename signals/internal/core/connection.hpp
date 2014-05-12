#pragma once

#include <cassert>
#include <future>
#include <vector>

namespace Signals {

template<typename Callback, typename ResultType>
class ConnectionList { //TODO: write interface
public:
	typedef NotVoid<ResultType> FutureResultType;
	Callback const callback;
	size_t const uid;
	std::vector<std::shared_future<FutureResultType>> futures;

	explicit ConnectionList(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}
};

}
