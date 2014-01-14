#pragma once

#include "../utils/threadpool.hpp"
#include <functional>
#include <future>


template<typename> class CallerSync;
template<typename> class CallerLazy;
template<typename> class CallerAsync;
template<typename> class CallerAuto;
template<typename> class CallerThread;
template<typename> class CallerThreadPool;

//synchronous calls
template<typename R, typename... Args>
class CallerSync<R(Args...)> {
public:
	std::shared_future<R> operator() (const std::function<R(Args...)> &callback, Args... args) {
		std::packaged_task<R(Args...)> task(callback);
		const std::shared_future<R> &f = task.get_future();
		task(args...);
		return f;
	}
};
template<typename... Args>
class CallerSync<void(Args...)> {
public:
	std::shared_future<int> operator() (const std::function<void(Args...)> &callback, Args... args) {
		auto closure = [=](Args... args)->int { callback(args...); return 0; };
		std::packaged_task<int(Args...)> task(closure);
		const std::shared_future<int> &f = task.get_future();
		task(args...);
		return f;
	}
};

//synchronous lazy calls
template<typename R, typename... Args>
class CallerLazy<R(Args...)> {
public:
	std::shared_future<R> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return std::async(std::launch::deferred, callback, args...);
	}
};
template<typename... Args>
class CallerLazy<void(Args...)> {
public:
	std::shared_future<int> operator() (const std::function<void(Args...)> &callback, Args... args) {
		auto closure = [=](Args... args)->int { callback(args...); return 0; };
		return std::async(std::launch::deferred, closure, args...);
	}
};

//asynchronous calls with std::launch::async (spawns a thread)
template<typename R, typename... Args>
class CallerAsync<R(Args...)> {
public:
	std::shared_future<R> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return std::async(std::launch::async, callback, args...);
	}
};
template<typename... Args>
class CallerAsync<void(Args...)> {
public:
	std::shared_future<int> operator() (const std::function<void(Args...)> &callback, Args... args) {
		auto closure = [=](Args... args)->int { callback(args...); return 0; };
		return std::async(std::launch::async, closure, args...);
	}
};

//asynchronous or synchronous calls at the runtime convenience
template<typename R, typename... Args>
class CallerAuto<R(Args...)> {
public:
	std::shared_future<R> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return std::async(std::launch::async | std::launch::deferred, callback, args...);
	}
};
template<typename... Args>
class CallerAuto<void(Args...)> {
public:
	std::shared_future<int> operator() (const std::function<void(Args...)> &callback, Args... args) {
		auto closure = [=](Args... args)->int { callback(args...); return 0; };
		return std::async(std::launch::async | std::launch::deferred, closure, args...);
	}
};

//tasks occur in a thread
template<typename R, typename... Args>
class CallerThread<R(Args...)> {
public:
	CallerThread() : threadPool(1) {
	}

	std::shared_future<R> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return threadPool.submit(callback, args...);
	}

private:
	Tests::ThreadPool threadPool;
};
template<typename... Args>
class CallerThread<void(Args...)> {
public:
	CallerThread() : threadPool(1) {
	}

	std::shared_future<int> operator() (const std::function<void(Args...)> &callback, Args... args) {
		std::function<int(Args...)> closure = [=](Args... args)->int { callback(args...); return 0; };
		return threadPool.submit(closure, args...);
	}

private:
	Tests::ThreadPool threadPool;
};

//tasks occur in the pool
template<typename R, typename... Args>
class CallerThreadPool<R(Args...)> {
public:
	CallerThreadPool() : threadPool(std::shared_ptr<Tests::ThreadPool>(new Tests::ThreadPool)) {
	}

	CallerThreadPool(std::shared_ptr<Tests::ThreadPool> threadPool) : threadPool(threadPool) {
	}

	std::shared_future<R> operator() (const std::function<R(Args...)> &callback, Args... args) {
		return threadPool->submit(callback, args...);
	}

private:
	std::shared_ptr<Tests::ThreadPool> threadPool;
};
template< typename... Args>
class CallerThreadPool<void(Args...)> {
public:
	CallerThreadPool() : threadPool(std::shared_ptr<Tests::ThreadPool>(new Tests::ThreadPool)) {
	}

	CallerThreadPool(std::shared_ptr<Tests::ThreadPool> threadPool) : threadPool(threadPool) {
	}

	std::shared_future<int> operator() (const std::function<void(Args...)> &callback, Args... args) {
		std::function<int(Args...)> closure = [=](Args... args)->int { callback(args...); return 0; };
		return threadPool->submit(closure, args...);
	}

private:
	std::shared_ptr<Tests::ThreadPool> threadPool;
};
