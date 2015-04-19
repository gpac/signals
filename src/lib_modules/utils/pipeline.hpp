#pragma once

#include <memory>
#include <vector>
#include "stranded_pool_executor.hpp"
#include "../core/module.hpp"
#include "helper.hpp"


#define EXECUTOR_SYNC ExecutorSync<void(Data)>
#define EXECUTOR_ASYNC StrandedPoolModuleExecutor
#define EXECUTOR EXECUTOR_ASYNC


namespace Modules {

struct IOutput;

struct ICompletionNotifier {
	virtual void finished() = 0;
};

struct IPipelinedModule : public IInputCap, public IOutputCap {
	virtual ~IPipelinedModule() noexcept(false) {}
	virtual void setSource(bool isSource) = 0;
	virtual bool isSource() const = 0;
	virtual bool isSink() const = 0;
	virtual void dispatch(Data data) = 0;
};

class PipelinedModule : public IPipelinedModule {
public:
	/* take ownership of module */
	PipelinedModule(Module *module, ICompletionNotifier *notify);
	~PipelinedModule () noexcept(false) {}

	template<typename OutputType>
	void connect(OutputType* output, size_t inputIdx) {
		ConnectModules(output, this, inputIdx, executor);
	}

	size_t getNumInputs() const override;
	IInput* getInput(size_t i) override;
	size_t getNumOutputs() const override;
	IOutput* getOutput(size_t i) const override;

	/* direct call: receiving nullptr stops the execution */
	void process(Data data);

	/* same as process() but uses the executor (may defer the call) */
	void dispatch(Data data);

	/* source modules are stopped manually - then the message propagates to other connected modules */
	void setSource(bool isSource);
	bool isSource() const;
	bool isSink() const;

private:
	void endOfStream();

	enum Type {
		None,
		Source
	};
	Type type;

	std::unique_ptr<Module> delegate;
	std::unique_ptr<IProcessExecutor> const localExecutor;
	IProcessExecutor &executor;
	ICompletionNotifier* const m_notify;
};

class Pipeline : public ICompletionNotifier {
public:
	Pipeline(bool isLowLatency = false);

	template<typename ModuleType>
	PipelinedModule* addModule(ModuleType* rawModule, bool isSource = false) { //Romain: we know isSource from the numInputs
		if (!rawModule)
			return nullptr;
		rawModule->setLowLatency(isLowLatency);
		auto module = uptr(new PipelinedModule(rawModule, this));
		module->setSource(isSource);
		auto ret = module.get();
		modules.push_back(std::move(module));
		return ret;
	}

	template<typename ModuleType>
	void connect(ModuleType* output, size_t outputIdx, PipelinedModule *input, size_t inputIdx) {
		if (input->isSink())
			numRemainingNotifications++;
		input->connect(output->getOutput(outputIdx), inputIdx);
	}

	void start();
	void waitForCompletion();
	void finished() override;

private:
	std::vector<std::unique_ptr<PipelinedModule>> modules;
	bool isLowLatency;

	std::mutex mutex;
	std::condition_variable condition;
	std::atomic<int> numRemainingNotifications;
};

}
