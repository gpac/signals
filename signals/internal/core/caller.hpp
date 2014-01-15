#pragma once

#include "../utils/threadpool.hpp"
#include "../utils/helper.hpp"
#include <functional>
#include <future>


namespace Signals {

template<typename> class CallerSync;
template<typename> class CallerLazy;
template<typename> class CallerAsync;
template<typename> class CallerAuto;
template<typename> class CallerThread;
template<typename> class CallerThreadPool;

template<typename R, typename... Args>
std::function<R(Args...)> NotVoidFunction(std::function<R(Args...)> fn) {
	return fn;
}

// converts a function returning void to a function return int (always 0).
template<typename... Args>
std::function<int(Args...)> NotVoidFunction(std::function<void(Args...)> fn) {
	auto closure = [=](Args... args) -> int {
	 	fn(args...);
	 	return 0;
 	};
	return closure;
}

//synchronous calls
template<typename R, typename... Args>
class CallerSync<R(Args...)> {
public:
	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &callback, Args... args) {
		std::packaged_task<NotVoid<R>(Args...)> task(NotVoidFunction(callback));
		const std::shared_future<NotVoid<R>> &f = task.get_future();
		task(args...);
		return f;
	}
};

//synchronous lazy calls
template<typename R, typename... Args>
class CallerLazy<R(Args...)> {
public:
	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return std::async(std::launch::deferred, NotVoidFunction(callback), args...);
	}
};

//asynchronous calls with std::launch::async (spawns a thread)
template<typename R, typename... Args>
class CallerAsync<R(Args...)> {
public:
	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return std::async(std::launch::async, NotVoidFunction(callback), args...);
	}
};

//asynchronous or synchronous calls at the runtime convenience
template<typename R, typename... Args>
class CallerAuto<R(Args...)> {
public:
	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return std::async(std::launch::async | std::launch::deferred, NotVoidFunction(callback), args...);
	}
};

//tasks occur in a thread
template<typename R, typename... Args>
class CallerThread<R(Args...)> {
public:
	CallerThread() : threadPool(1) {
	}

	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return threadPool.submit(NotVoidFunction(callback), args...);
	}

private:
	ThreadPool threadPool;
};

//tasks occur in the pool
template<typename R, typename... Args>
class CallerThreadPool<R(Args...)> {
public:
	CallerThreadPool() : threadPool(std::shared_ptr<ThreadPool>(new ThreadPool)) {
	}

	CallerThreadPool(std::shared_ptr<ThreadPool> threadPool) : threadPool(threadPool) {
	}

	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return threadPool->submit(NotVoidFunction(callback), args...);
	}

private:
	std::shared_ptr<ThreadPool> threadPool;
};

}
