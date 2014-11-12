#pragma once

#define ASIO_STANDALONE
#include <asio/asio.hpp>
#include "../../../signals/internal/core/executor.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>


using namespace Signals;

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

namespace asio {
class thread_pool;
template<typename> class strand;
}

namespace Modules {

class Data;

typedef IExecutor<void(std::shared_ptr<Data>)> IProcessExecutor;

//tasks occur in the default thread pool
//when tasks belong to a strand, they are processed non-concurrently in FIFO order
class StrandedPoolModuleExecutor : public IProcessExecutor {
public:
	StrandedPoolModuleExecutor(); //uses the default Modules thread pool
	StrandedPoolModuleExecutor(asio::thread_pool &threadPool);
	~StrandedPoolModuleExecutor();
	std::shared_future<NotVoid<void>> operator() (const std::function<void(std::shared_ptr<Data>)> &fn, std::shared_ptr<Data>);

private:
	asio::thread_pool &threadPool;
	asio::strand<asio::thread_pool::executor_type> strand;
};

static ExecutorSync<void(std::shared_ptr<Data>)> defaultExecutor;
//TODO: static StrandedPoolModuleExecutor defaultExecutor;

}
