#pragma once

#include <cassert>
#include <future>


template<typename Callback, typename ResultType>
class Connection {
public:
	Callback callback;
	std::shared_future<ResultType> future;
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
	std::shared_future<int> future;
	size_t uid;

	explicit Connection(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}

	~Connection() {
	}
};
