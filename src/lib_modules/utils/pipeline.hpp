#pragma once

#include <memory>
#include <vector>
#include "stranded_pool_executor.hpp"
#include "../core/module.hpp"

namespace Modules {

struct IOutput;

struct ICompletionNotifier {
	virtual void finished() = 0;
};

struct IPipelinedModule : public IOutputCap {
	virtual ~IPipelinedModule() {}
	virtual void setSource(bool isSource) = 0;
	virtual bool isSource() const = 0;
	virtual bool isSink() const = 0;
	virtual void dispatch(Data data) = 0;
};

template<typename ModuleType>
class PipelinedModule : public IPipelinedModule, public IModule {
public:
	/* take ownership of module */
	PipelinedModule(ModuleType *module, ICompletionNotifier *notify);
	void connect(IOutput* out);
	size_t getNumOutputs() const override;
	IOutput* getOutput(size_t i) const override;

	/* direct call: receiving nullptr stops the execution */
	void process(Data data);

	/* same as process() but uses the executor (may defer the call) */
	void dispatch(Data data) override;

	/* source modules are stopped manually - then the message propagates to other connected modules */
	void setSource(bool isSource) override;
	bool isSource() const override;
	bool isSink() const override;

private:
	void endOfStream();

	enum Type {
		None,
		Source
	};
	Type type;

	std::unique_ptr<ModuleType> delegate;
	std::unique_ptr<IProcessExecutor> const localExecutor;
	IProcessExecutor &executor;
	ICompletionNotifier* const m_notify;
};

class Pipeline : public ICompletionNotifier {
public:
	Pipeline(bool isLowLatency = false);

	template<typename ModuleType>
	PipelinedModule<ModuleType>* addModule(ModuleType* rawModule, bool isSource = false);

	template<typename ModuleType>
	void connect(IOutput* out, PipelinedModule<ModuleType> *module);

	void start();
	void waitForCompletion();
	void finished() override;

private:
	std::vector<std::unique_ptr<IPipelinedModule>> modules;
	bool isLowLatency;

	std::mutex mutex;
	std::condition_variable condition;
	std::atomic<int> numRemainingNotifications;
};

}
