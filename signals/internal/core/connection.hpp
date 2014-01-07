#pragma once

#include <cassert>
#include <future>

#include "../utils/queue.hpp"


template<typename Callback, typename ResultType>
class ConnectionQueue {
public:
	Callback callback;
	Queue<std::shared_future<ResultType>> futures;
	size_t uid;

	explicit ConnectionQueue(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}
};

//specialized for void
template<typename Callback>
class ConnectionQueue<Callback, void> {
public:
	Callback callback;
	Queue<std::shared_future<int>> futures;
	size_t uid;

	explicit ConnectionQueue(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}
};


template<typename Callback, typename ResultType>
class ConnectionQueueThreadSafe {
public:
	Callback callback;
	QueueThreadSafe<std::shared_future<ResultType>> futures;
	size_t uid;

	explicit ConnectionQueueThreadSafe(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}
};

//specialized for void
template<typename Callback>
class ConnectionQueueThreadSafe<Callback, void> {
public:
	Callback callback;
	QueueThreadSafe<std::shared_future<int>> futures;
	size_t uid;

	explicit ConnectionQueueThreadSafe(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}
};
