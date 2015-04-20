#include "pipeline.hpp"


#define EXECUTOR_SYNC ExecutorSync<void(Data)>
#define EXECUTOR_ASYNC StrandedPoolModuleExecutor
#define EXECUTOR EXECUTOR_SYNC //Romain


namespace Modules {

class PipelinedInput : public IInput {
public:
	PipelinedInput(PipelinedModule * const module, IInput *input) : delegate(input), module(module) {}
	virtual ~PipelinedInput() noexcept(false) {}

	/* direct call: receiving nullptr stops the execution */
	virtual void process(Data data) override {
		if (data) {
			delegate->process(data);
		} else {
			module->endOfStream();
		}
	}

private:
	IInput *delegate;
	PipelinedModule * const module;
};

PipelinedModule::PipelinedModule(Module *module, ICompletionNotifier *notify)
: delegate(module), localExecutor(new EXECUTOR), executor(*localExecutor), m_notify(notify) {
}

void PipelinedModule::mimicInputs() {
	auto const delegateInputs = delegate->getNumInputs();
	auto const thisInputs = inputs.size();
	if (thisInputs < delegateInputs) {
		for (size_t i = thisInputs; i < delegateInputs; ++i) {
			addInput(new PipelinedInput(this, delegate->getInput(i)));
		}
	}
}

size_t PipelinedModule::getNumInputs() const {
	return delegate->getNumInputs();
}

IInput* PipelinedModule::getInput(size_t i) {
	mimicInputs();
	return inputs[i].get();
}

size_t PipelinedModule::getNumOutputs() const {
	return delegate->getNumOutputs();
}

IOutput* PipelinedModule::getOutput(size_t i) const {
	return delegate->getOutput(i);
}

bool PipelinedModule::isSource() const {
	return delegate->getNumInputs() == 0;
}

bool PipelinedModule::isSink() const {
	return delegate->getNumOutputs() == 0;
}

void PipelinedModule::dispatch(Data data) {
	if (isSource()) {
		assert(data == nullptr);
		assert(getNumInputs() == 0);
		delegate->addInput(new Input<DataBase>(delegate.get()));
		executor(MEMBER_FUNCTOR_PROCESS(delegate->getInput(0)), data);
	}

	for (size_t i = 0; i < getNumInputs(); ++i)
		executor(MEMBER_FUNCTOR_PROCESS(getInput(i)), data);
}

void PipelinedModule::endOfStream() {
	delegate->flush();
	if (isSink()) {
		m_notify->finished();
	} else {
		for (size_t i = 0; i < delegate->getNumOutputs(); ++i)
			delegate->getOutput(i)->emit(Data(nullptr));
	}
}

Pipeline::Pipeline(bool isLowLatency) : isLowLatency(isLowLatency), numRemainingNotifications(0) {
}

PipelinedModule* Pipeline::addModule(Module* rawModule) {
	if (!rawModule)
		return nullptr;
	rawModule->setLowLatency(isLowLatency);
	auto module = uptr(new PipelinedModule(rawModule, this));
	auto ret = module.get();
	modules.push_back(std::move(module));
	return ret;
}

void Pipeline::start() {
	for (auto &m : modules) {
		if (m->isSource())
			m->dispatch(nullptr);
	}
}

void Pipeline::waitForCompletion() {
	std::unique_lock<std::mutex> lock(mutex);
	while (numRemainingNotifications > 0) {
		condition.wait(lock);
	}
}

void Pipeline::finished() {
	std::unique_lock<std::mutex> lock(mutex);
	assert(numRemainingNotifications > 0);
	--numRemainingNotifications;
	condition.notify_one();
}

}
