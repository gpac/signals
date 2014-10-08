#pragma once

#define ASIO_STANDALONE
#include <asio/asio.hpp>
#include "../../../signals/internal/core/executor.hpp"
#include <memory>


using namespace Signals;

namespace asio {
class thread_pool;
template<typename> class strand;
}

namespace Modules {

class Data;

typedef IExecutor<void(std::shared_ptr<Data>)> IProcessExecutor;

//tasks occur in the default thread pool
//when tasks belong to a strand, they are processed non-concurrently in FIFO order
class StrandedPoolModuleExecutor : IProcessExecutor {
public:
	StrandedPoolModuleExecutor(); //uses the default Modules thread pool
	StrandedPoolModuleExecutor(asio::thread_pool &threadPool);
	std::shared_future<NotVoid<void>> operator() (const std::function<void(std::shared_ptr<Data>)> &fn, std::shared_ptr<Data>);

private:
	asio::thread_pool &threadPool;
	asio::strand<asio::thread_pool::executor_type> strand;
};

static ExecutorSync<void(std::shared_ptr<Data>)> defaultExecutor;
//TODO: static StrandedPoolModuleExecutor defaultExecutor;

}
