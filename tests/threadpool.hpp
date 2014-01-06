#pragma once

#include <queue>


namespace {
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

	private:
		mutable std::mutex mutex;
		std::queue<T> dataQueue;
		std::condition_variable dataCondition;
	};
}


namespace Tests { //TODO: create a util namespace?
	class ThreadPool {
	public:
		ThreadPool() {
			done = false;
			waitAndExit = false;
			const unsigned threadCount = std::thread::hardware_concurrency();
			for (unsigned i = 0; i < threadCount; ++i) {
				threads.push_back(std::thread(&ThreadPool::run, this));
			}
		}

		~ThreadPool() {
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

		template<typename Signature>
		void submit(std::function<Signature> &f)	{
			workQueue.push(f);
		}

	private:
		void run() {
			while (!done) {
				std::function<void()> task;
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
		ThreadSafeQueue<std::function<void()>> workQueue;
		std::vector<std::thread> threads;
	};
}
