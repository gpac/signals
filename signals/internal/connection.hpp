#pragma once

#include <cassert>
#include <future>
#include <queue>


template<typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() {
	}

	void push(T data) {
		std::lock_guard<std::mutex> lock(mutex);
		dataQueue.push(std::move(data));
		dataCondition.notify_one();
	}

	bool tryPop(T &value) {
		std::lock_guard<std::mutex> lock(mutex);
		if (dataQueue.empty()) {
			return false;
		}
		value = std::move(dataQueue.front());
		dataQueue.pop();
		return true;
	}

	std::shared_ptr<T> tryPop() {
		std::lock_guard<std::mutex> lock(mutex);
		if (dataQueue.empty()) {
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> res(std::make_shared<T>(std::move(dataQueue.front())));
		dataQueue.pop();
		return res;
	}

	bool empty() const {
		std::lock_guard<std::mutex> lock(mutex);
		return dataQueue.empty();
	}

	void clear() {
		std::lock_guard<std::mutex> lock(mutex);
		while (!dataQueue.empty()) {
			dataQueue.pop();
		}
	}

private:
	mutable std::mutex mutex;
	std::queue<T> dataQueue;
	std::condition_variable dataCondition;
};

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
