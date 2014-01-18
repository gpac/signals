#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include "queue.hpp"


namespace Signals {

class ThreadPool {
public:
	ThreadPool(const unsigned threadCount = std::thread::hardware_concurrency()) {
		done = false;
		waitAndExit = false;
		for (unsigned i = 0; i < threadCount; ++i) {
			threads.push_back(std::thread(&ThreadPool::run, this));
		}
	}

	~ThreadPool() {
		WaitForCompletion();
		done = true;
	}

	void WaitForCompletion() {
		waitAndExit = true;
		for (size_t i = 0; i < threads.size(); ++i) {
			if (threads[i].joinable()) {
				threads[i].join();
			}
		}
	}

	template<typename Callback, typename... Args>
	std::shared_future<Callback> submit(const std::function<Callback(Args...)> &callback, Args... args)	{
#if 0 //FIXME: better but crashes
		std::packaged_task<Callback(Args...)> task(callback);
		const std::shared_future<Callback> &future = task.get_future();
		std::function<void(void)> f = [&task, args...]() {
			task(args...);
		};
		workQueue.push(f);
		return future;
#endif
		const std::shared_future<Callback> &future = std::async(std::launch::deferred, callback, args...);
		std::function<void(void)> f = [future]() {
			future.get();
		};
		workQueue.push(f);
		return future;
	}

private:
	ThreadPool(const ThreadPool&) = delete;

	void run() {
		while (!done) {
			std::function<void(void)> task;
			if (workQueue.tryPop(task)) {
				task();
			} else {
				std::this_thread::yield();
				if (waitAndExit) {
					done = true;
				}
			}
		}
	}

	std::atomic_bool done, waitAndExit;
	QueueThreadSafe<std::function<void(void)>> workQueue;
	std::vector<std::thread> threads;
};
}
