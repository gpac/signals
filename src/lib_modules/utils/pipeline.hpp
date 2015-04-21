#pragma once

#include <memory>
#include <vector>
#include "stranded_pool_executor.hpp"
#include "../core/module.hpp"
#include "helper.hpp"


namespace Modules {

struct IOutput;

struct ICompletionNotifier {
	virtual void finished() = 0;
};

struct IPipelineModule {
	virtual bool isSource() const = 0;
	virtual bool isSink() const = 0;
	virtual void dispatch(Data data) = 0;
};

class PipelinedModule : public ICompletionNotifier, public IPipelineModule, public InputCap {
public:
	/* take ownership of module */
	PipelinedModule(Module *module, ICompletionNotifier *notify);
	~PipelinedModule () noexcept(false) {}

	template<typename OutputType>
	void connect(OutputType* output, size_t inputIdx) {
		ConnectOutputToInput(output, getInput(inputIdx), executor);
	}

	size_t getNumInputs() const override;
	IInput* getInput(size_t i) override;
	size_t getNumOutputs() const;
	IOutput* getOutput(size_t i) const;

	/* source modules are stopped manually - then the message propagates to other connected modules */
	bool isSource() const override;
	bool isSink() const override;

private:
	void mimicInputs();

	/* same as process() but uses the executor (may defer the call) */
	void dispatch(Data data) override;
	void finished() override;

	std::unique_ptr<Module> delegate;
	std::unique_ptr<IProcessExecutor> const localExecutor;
	IProcessExecutor &executor;
	ICompletionNotifier* const m_notify;
};

class Pipeline : public ICompletionNotifier {
public:
	Pipeline(bool isLowLatency = false);
	PipelinedModule* addModule(Module* rawModule);

	template<typename ModuleType>
	void connect(ModuleType* output, size_t outputIdx, PipelinedModule *input, size_t inputIdx) {
		if (input->isSink())
			numRemainingNotifications++;
		input->connect(output->getOutput(outputIdx), inputIdx);
	}

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
