#pragma once

#include <cassert>
#include <future>
#include <queue>


template<typename Callback, typename ResultType>
class Connection {
public:
	Callback callback;
	ThreadSafeQueue<std::shared_future<ResultType>> futures;
	size_t uid;

	explicit Connection(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}

	~Connection() {
	}
};

//specialized for void
template<typename Callback>
class Connection<Callback, void> {
public:
	Callback callback;
	ThreadSafeQueue<std::shared_future<int>> futures;
	size_t uid;

	explicit Connection(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}

	~Connection() {
	}
};
