#pragma once

#include <memory>
#include <vector>
#include "stranded_pool_executor.hpp"
#include "../core/module.hpp"


#define EXECUTOR_SYNC ExecutorSync<void(Data)>
#define EXECUTOR_ASYNC StrandedPoolModuleExecutor
#define EXECUTOR EXECUTOR_ASYNC


namespace Modules {

struct IOutput;

struct ICompletionNotifier {
	virtual void finished() = 0;
};

struct IPipelinedModule : public IOutputCap, public IInputCap {
	virtual ~IPipelinedModule() {}
	virtual void setSource(bool isSource) = 0;
	virtual bool isSource() const = 0;
	virtual bool isSink() const = 0;
	virtual void dispatch(Data data) = 0;
};

template<typename Result, typename Class>
MemberFunctor<Result, Class, Result(Class::*)(Data)>
MEMBER_FUNCTOR(Class* objectPtr, Result(Class::*memberFunction) (Data)) {
	return MemberFunctor<Result, Class, Result(Class::*)(Data)>(objectPtr, memberFunction);
}

template<typename ModuleType>
class PipelinedModule : public IPipelinedModule, public ModuleS {
public:
	/* take ownership of module */
	PipelinedModule(ModuleType *module, ICompletionNotifier *notify)
	: type(None), delegate(module), localExecutor(new EXECUTOR), executor(*localExecutor), m_notify(notify) {
	}
	void connect(IOutput* out) {
		ConnectOutputToModule(out, this, executor);
	}
	size_t getNumInputs() const override {
		return delegate->getNumInputs();
	}
	IInput* getInput(size_t i) override {
		return delegate->getInput(i);
	}
	size_t getNumOutputs() const override {
		return delegate->getNumOutputs();
	}
	IOutput* getOutput(size_t i) const override {
		return delegate->getOutput(i);
	}

	/* direct call: receiving nullptr stops the execution */
	void process(Data data) {
		if (data) {
			delegate->process(data);
		} else {
			endOfStream();
		}
	}

	/* same as process() but uses the executor (may defer the call) */
	void dispatch(Data data) override {
		if (isSource()) {
			assert(data == nullptr);
			executor(MEMBER_FUNCTOR(delegate.get(), &ModuleType::process), data);
		}
		executor(MEMBER_FUNCTOR(this, &PipelinedModule::process), data);
	}

	/* source modules are stopped manually - then the message propagates to other connected modules */
	void setSource(bool isSource) override {
		type = isSource ? Source : None;
	}
	bool isSource() const override {
		return type == Source;
	}
	bool isSink() const override {
		return delegate->getNumOutputs() == 0;
	}

private:
	void endOfStream() {
		delegate->flush();
		if (isSink()) {
			m_notify->finished();
		} else {
			for (size_t i = 0; i < delegate->getNumOutputs(); ++i)
				delegate->getOutput(i)->emit(Data(nullptr));
		}
	}

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
	PipelinedModule<ModuleType>* addModule(ModuleType* rawModule, bool isSource = false) {
		if (!rawModule)
			return nullptr;
		rawModule->setLowLatency(isLowLatency);
		auto module = uptr(new PipelinedModule<ModuleType>(rawModule, this));
		module->setSource(isSource);
		auto ret = module.get();
		modules.push_back(std::move(module));
		return ret;
	}

	template<typename ModuleType>
	void connect(IOutput* out, PipelinedModule<ModuleType> *module) {
		if (module->isSink())
			numRemainingNotifications++;
		module->connect(out);
	}

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
