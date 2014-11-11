#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>


namespace Signals {

//FIXME: it doesn't compile with non-pointer types, see ENABLE_FAILING_TESTS
template<typename T>
class QueueThreadSafe {
public:
	QueueThreadSafe() {
	}

	void push(T data) {
		std::lock_guard<std::mutex> lock(mutex);
		dataQueue.push(std::move(data));
		dataAvailable.notify_one();
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
		while (dataQueue.empty())
			dataAvailable.wait(lock);
		T p;
		std::swap(p, dataQueue.front());
		dataQueue.pop();
		return p;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(mutex);
		std::queue<T> emptyQueue;
		std::swap(emptyQueue, dataQueue);
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
	std::condition_variable dataAvailable;
};

}
