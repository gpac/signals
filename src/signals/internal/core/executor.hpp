#pragma once

#include "../utils/threadpool.hpp"
#include "../utils/helper.hpp"
#include <functional>
#include <future>


namespace Signals {

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

template<typename> class IExecutor;

template <typename R, typename... Args>
class IExecutor<R(Args...)> {
public:
	virtual ~IExecutor() {}
	virtual std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &fn, Args... args) = 0;
};

template<typename> class ExecutorSync;
template<typename> class ExecutorLazy;
template<typename> class ExecutorAsync;
template<typename> class ExecutorAuto;
template<typename> class ExecutorThread;
template<typename> class ExecutorThreadPool;

//synchronous calls
template<typename R, typename... Args>
class ExecutorSync<R(Args...)> : public IExecutor<R(Args...)> {
public:
	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &fn, Args... args) {
		std::packaged_task<NotVoid<R>(Args...)> task(NotVoidFunction(fn));
		const std::shared_future<NotVoid<R>> &f = task.get_future();
		task(args...);
		return f;
	}
};

//synchronous lazy calls
template<typename R, typename... Args>
class ExecutorLazy<R(Args...)> : public IExecutor<R(Args...)> {
public:
	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &fn, Args... args) {
		return std::async(std::launch::deferred, NotVoidFunction(fn), args...);
	}
};

//asynchronous calls with std::launch::async (spawns a thread)
template<typename R, typename... Args>
class ExecutorAsync<R(Args...)> : public IExecutor<R(Args...)> {
public:
	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &fn, Args... args) {
		return std::async(std::launch::async, NotVoidFunction(fn), args...);
	}
};

//asynchronous or synchronous calls at the runtime convenience
template<typename R, typename... Args>
class ExecutorAuto<R(Args...)> : public IExecutor<R(Args...)> {
public:
	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &fn, Args... args) {
		return std::async(std::launch::async | std::launch::deferred, NotVoidFunction(fn), args...);
	}
};

//tasks occur in a thread
template<typename R, typename... Args>
class ExecutorThread<R(Args...)> : public IExecutor<R(Args...)> {
public:
	ExecutorThread() : threadPool(1) {
	}

	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &fn, Args... args) {
		return threadPool.submit(NotVoidFunction(fn), args...);
	}

private:
	ThreadPool threadPool;
};

//tasks occur in the pool
template<typename R, typename... Args>
class ExecutorThreadPool<R(Args...)> : public IExecutor<R(Args...)> {
public:
	ExecutorThreadPool() : threadPool(std::shared_ptr<ThreadPool>(new ThreadPool)) {
	}

	ExecutorThreadPool(std::shared_ptr<ThreadPool> threadPool) : threadPool(threadPool) {
	}

	std::shared_future<NotVoid<R>> operator() (const std::function<R(Args...)> &fn, Args... args) {
		return threadPool->submit(NotVoidFunction(fn), args...);
	}

private:
	std::shared_ptr<ThreadPool> threadPool;
};

}
