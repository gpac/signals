#pragma once

#include <memory>
#include <vector>
#include "stranded_pool_executor.hpp"
#include "../core/module.hpp"
#include "helper.hpp"


namespace Pipelines {

struct ICompletionNotifier {
	virtual void finished() = 0;
};

struct IPipelineModule {
	virtual bool isSource() const = 0;
	virtual bool isSink() const = 0;
	virtual void dispatch(Modules::Data data) = 0;
	virtual void connect(Modules::IOutput *output, size_t inputIdx) = 0;
	virtual Modules::IOutput* getOutput(size_t i) const = 0;
};

class PipelinedModule : public ICompletionNotifier, public IPipelineModule, public Modules::InputCap {
public:
	/* take ownership of module */
	PipelinedModule(Modules::Module *module, ICompletionNotifier *notify);
	~PipelinedModule () noexcept(false) {}

	size_t getNumInputs() const override;
	size_t getNumOutputs() const;
	Modules::IOutput* getOutput(size_t i) const;

	/* source modules are stopped manually - then the message propagates to other connected modules */
	bool isSource() const override;
	bool isSink() const override;

private:
	virtual void connect(Modules::IOutput *output, size_t inputIdx) override;
	Modules::IInput* getInput(size_t i) override;
	void mimicInputs();

	/* same as process() but uses the executor (may defer the call) */
	void dispatch(Modules::Data data) override;
	void finished() override;

	std::unique_ptr<Modules::Module> delegate;
	std::unique_ptr<Modules::IProcessExecutor> const localExecutor;
	Modules::IProcessExecutor &executor;
	ICompletionNotifier* const m_notify;
};

class Pipeline : public ICompletionNotifier {
public:
	Pipeline(bool isLowLatency = false);
	PipelinedModule* addModule(Modules::Module* rawModule);
	void connect(IPipelineModule *prev, size_t outputIdx, IPipelineModule *next, size_t inputIdx);
	void start();
	void waitForCompletion();

private:
	void finished() override;

	std::vector<std::unique_ptr<IPipelineModule>> modules;
	bool isLowLatency;

	std::mutex mutex;
	std::condition_variable condition;
	std::atomic<int> numRemainingNotifications;
};

}
