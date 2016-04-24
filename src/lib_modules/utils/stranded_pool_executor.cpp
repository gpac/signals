#include "stranded_pool_executor.hpp"
#include <functional>
#include <future>


namespace Modules {

//a shared thread pool for the modules
#define N_THREADS_PER_CPU 2
static asio::thread_pool g_threadPool { N_THREADS_PER_CPU * std::thread::hardware_concurrency() };

StrandedPoolModuleExecutor::StrandedPoolModuleExecutor() : strand(g_threadPool.get_executor()) {
}

StrandedPoolModuleExecutor::StrandedPoolModuleExecutor(asio::thread_pool &threadPool) : strand(threadPool.get_executor()) {
}

std::shared_future<NotVoid<void>> StrandedPoolModuleExecutor::operator() (const std::function<void()> &fn) {
	std::shared_future<NotVoid<void>> future = std::async(std::launch::deferred, [] { return NotVoid<void>(); });
	auto closure = [future, fn]() -> void {
		fn();
		future.get();
	};
	asio::post(strand, closure);
	return future;
}

}
