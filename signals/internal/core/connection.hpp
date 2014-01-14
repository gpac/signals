#pragma once

#include <cassert>
#include <future>
#include "../utils/queue.hpp"

template<typename Callback, typename ResultType>
class ConnectionQueue {
public:
	typedef NotVoid<ResultType> FutureResultType;
	Callback callback;
	Queue<std::shared_future<FutureResultType>> futures;
	size_t uid;

	explicit ConnectionQueue(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}
};

template<typename Callback, typename ResultType>
class ConnectionQueueThreadSafe {
public:
	typedef NotVoid<ResultType> FutureResultType;
	Callback callback;
	QueueThreadSafe<std::shared_future<FutureResultType>> futures;
	size_t uid;

	explicit ConnectionQueueThreadSafe(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}
};

