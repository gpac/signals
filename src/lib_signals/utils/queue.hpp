#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>


namespace Signals {

template<typename T>
class Queue {
public:
	Queue() {}
	virtual ~Queue() noexcept(false) {}

	virtual void push(T data) {
		std::lock_guard<std::mutex> lock(mutex);
		pushUnsafe(data);
	}

	virtual bool tryPop(T &value) {
		std::lock_guard<std::mutex> lock(mutex);
		if (dataQueue.empty()) {
			return false;
		}
		value = std::move(dataQueue.front());
		dataQueue.pop();
		return true;
	}

	virtual T pop() {
		std::unique_lock<std::mutex> lock(mutex);
		while (dataQueue.empty())
			dataAvailable.wait(lock);
		T p;
		std::swap(p, dataQueue.front());
		dataQueue.pop();
		return p;
	}

	virtual void clear() {
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

protected:
	void pushUnsafe(T data) {
		dataQueue.push(std::move(data));
		dataAvailable.notify_one();
	}

	mutable std::mutex mutex;
	std::queue<T> dataQueue;
	std::condition_variable dataAvailable;

private:
	Queue(const Queue&) = delete;
	Queue& operator= (const Queue&) = delete;
};

template<typename T>
class QueueMaxSize : public Queue<T> {
public:
	QueueMaxSize(size_t maxSize = std::numeric_limits<size_t>::max()) : maxSize(maxSize) {
		if (maxSize == 0)
			throw std::runtime_error("QueueMaxSize size cannot be 0.");
	}
	virtual ~QueueMaxSize() noexcept(false) {}

	bool tryPush(T data) {
		std::lock_guard<std::mutex> lock(mutex);
		if (dataQueue.size() < maxSize) {
			pushUnsafe(data);
			dataAvailable.notify_one();
			return true;
		} else {
			return false;
		}
	}

	void push(T data) {
		std::unique_lock<std::mutex> lock(mutex);
		while (dataQueue.size() > maxSize)
			dataPopped.wait(lock);
		Queue::pushUnsafe(data);
	}

	T pop() {
		T p = Queue::pop();
		dataPopped.notify_one();
		return p;
	}

	/* After a clear() call, you are guaranteed that all blocking push() will
	   awaken. Data will be effectively pushed and it is your reponsability to
	   pop() them if necessary (e.g. when more pushers than the queue size) */
	virtual void clear() {
		dataPopped.notify_all();
		Queue::clear();
	}

private:
	QueueMaxSize(const QueueMaxSize&) = delete;
	QueueMaxSize& operator= (const QueueMaxSize&) = delete;

	size_t maxSize;
	std::condition_variable dataPopped;
};

}
