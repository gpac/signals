#pragma once

#include <cassert>
#include <future>
#include <list>

namespace Signals {

template<typename Callback, typename ResultType>
class ConnectionList { //TODO: write interface
public:
	typedef NotVoid<ResultType> FutureResultType;
	Callback callback;
	std::list<std::shared_future<FutureResultType>> futures;
	size_t uid;

	explicit ConnectionList(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}
};

}
