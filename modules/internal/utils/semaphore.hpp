#pragma once

#include <condition_variable>
#include <mutex>


class Semaphore {
public:
	Semaphore(int initialCount = 1) : count(initialCount) {
	}

	void notify() {
		std::unique_lock<std::mutex> lock(mutex);
		++count;
		condition.notify_one();
	}

	void wait() {
		std::unique_lock<std::mutex> lock(mutex);
		while (count == 0) {
			condition.wait(lock);
		}
		--count;
	}

	bool wait_for(unsigned timeOutMs) {
		std::chrono::milliseconds timeDur(timeOutMs);
		std::unique_lock<std::mutex> lock(mutex);
		while (count == 0) {
			if (condition.wait_for(lock, timeDur) == std::cv_status::timeout) {
				return false;
			}
		}
		--count;
		return true;
	}

private:
	volatile int count;
	std::mutex mutex;
	std::condition_variable condition;
};