#pragma once

#define ASIO_STANDALONE
#include <asio/asio.hpp>
#include "../core/data.hpp"
#include "lib_signals/core/executor.hpp"
#include <memory>


namespace asio {
class thread_pool;
template<typename> class strand;
}

namespace Modules {

typedef Signals::IExecutor<void(Data)> IProcessExecutor;

//tasks occur in the default thread pool
//when tasks belong to a strand, they are processed non-concurrently in FIFO order
class StrandedPoolModuleExecutor : public IProcessExecutor {
public:
	StrandedPoolModuleExecutor();
	StrandedPoolModuleExecutor(asio::thread_pool &threadPool);
	std::shared_future<NotVoid<void>> operator() (const std::function<void(Data)> &fn, Data);

private:
	asio::strand<asio::thread_pool::executor_type> strand;
};

static Signals::ExecutorSync<void(Data)> g_executorSync;
//static StrandedPoolModuleExecutor g_StrandedExecutor;
#define defaultExecutor g_executorSync

}
