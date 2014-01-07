#pragma once

#include <atomic>
#include <functional>
#include <thread>

#include "queue.hpp"


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
				std::function<void()> task; //TODO: set signature
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
		QueueThreadSafe<std::function<void()>> workQueue;
		std::vector<std::thread> threads;
	};
}
