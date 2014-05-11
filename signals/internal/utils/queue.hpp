#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>


namespace Signals {

template<typename T>
class QueueThreadSafe {
public:
	QueueThreadSafe() {
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

	T tryPop() {
		std::lock_guard<std::mutex> lock(mutex);
		if (dataQueue.empty()) {
			return nullptr;
		}
		auto res = T(std::move(dataQueue.front()));
		dataQueue.pop();
		return res;
	}

	T pop() {
		std::unique_lock<std::mutex> lock(mutex);
		while(dataQueue.empty())
			dataCondition.wait(lock);

		T p;
		std::swap(p, dataQueue.front());
		dataQueue.pop();
		return p;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(mutex);
		while (!dataQueue.empty()) {
			dataQueue.pop();
		}
	}

#ifdef TESTS
	size_t size() const {
		std::lock_guard<std::mutex> lock(mutex);
		return dataQueue.size();
	}

	T& operator[] (size_t index) {
		std::lock_guard<std::mutex> lock(mutex);
		const size_t dataSize = dataQueue.size();
		assert(index < dataSize);
		if (index == 0) {
			return dataQueue.front();
		}
		std::queue<T> tmpQueue;
		for (size_t i = 0; i < index; ++i) {
			tmpQueue.push(dataQueue.front());
			dataQueue.pop();
		}
		T &res = dataQueue.front();
		for (size_t i = index; i < dataSize; ++i) {
			tmpQueue.push(dataQueue.front());
			dataQueue.pop();
		}
		assert((dataQueue.size() == 0) && (tmpQueue.size() == dataSize));
		for (size_t i = 0; i < dataSize; ++i) {
			dataQueue.push(tmpQueue.front());
			tmpQueue.pop();
		}
		return res;
	}
#endif

private:
	QueueThreadSafe(const QueueThreadSafe&) = delete;
	QueueThreadSafe& operator= (const QueueThreadSafe&) = delete;

	mutable std::mutex mutex;
	std::queue<T> dataQueue;
	std::condition_variable dataCondition;
};

template<typename T>
class Queue : public std::queue<T> {
	std::shared_ptr<T> tryPop() {
		if (std::queue<T>::empty()) {
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> res(std::make_shared<T>(std::move(std::queue<T>::front())));
		std::queue<T>::pop();
		return res;
	}
};

}
