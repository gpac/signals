#include "stranded_pool_executor.hpp"
#include <functional>
#include <future>


namespace Modules {

//a shared thread pool for the modules
#define N_THREADS_PER_CPU 1 //TODO: run perf tests - some modules may be blocking somehow? (I/O, own parallelisme pattern as in x264, etc.)
static asio::thread_pool g_threadPool { N_THREADS_PER_CPU * std::thread::hardware_concurrency() };

StrandedPoolModuleExecutor::StrandedPoolModuleExecutor() : strand(g_threadPool.get_executor()) {
}

StrandedPoolModuleExecutor::StrandedPoolModuleExecutor(asio::thread_pool &threadPool) : strand(threadPool.get_executor()) {
}

StrandedPoolModuleExecutor::~StrandedPoolModuleExecutor() {
}

std::shared_future<NotVoid<void>> StrandedPoolModuleExecutor::operator() (const std::function<void(std::shared_ptr<const Data>)> &fn, std::shared_ptr<const Data> data) {
	std::shared_future<NotVoid<void>> future = std::async(std::launch::deferred, [] { return NotVoid<void>(); });
	auto closure = [future, fn, data]() -> void {
		fn(data);
		future.get();
	};
	asio::post(strand, closure);
	return future;
}

}