#pragma once

#define ASIO_STANDALONE
#include <asio/asio.hpp>
#include "lib_signals/internal/core/executor.hpp"
#include <memory>


using namespace Signals;

namespace asio {
class thread_pool;
template<typename> class strand;
}

namespace Modules {

class Data;

typedef IExecutor<void(std::shared_ptr<const Data>)> IProcessExecutor;

//tasks occur in the default thread pool
//when tasks belong to a strand, they are processed non-concurrently in FIFO order
class StrandedPoolModuleExecutor : public IProcessExecutor {
public:
	StrandedPoolModuleExecutor();
	StrandedPoolModuleExecutor(asio::thread_pool &threadPool);
	std::shared_future<NotVoid<void>> operator() (const std::function<void(std::shared_ptr<const Data>)> &fn, std::shared_ptr<const Data>);

private:
	asio::strand<asio::thread_pool::executor_type> strand;
};

static ExecutorSync<void(std::shared_ptr<const Data>)> g_executorSync;
//static StrandedPoolModuleExecutor g_StrandedExecutor;
#define defaultExecutor g_executorSync

}
