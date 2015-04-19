#include "pipeline.hpp"


#define EXECUTOR_SYNC ExecutorSync<void(Data)>
#define EXECUTOR_ASYNC StrandedPoolModuleExecutor
#define EXECUTOR EXECUTOR_SYNC //Romain


namespace Modules {

PipelinedModule::PipelinedModule(Module *module, ICompletionNotifier *notify)
: delegate(module), localExecutor(new EXECUTOR), executor(*localExecutor), m_notify(notify) {
}

size_t PipelinedModule::getNumInputs() const {
	return delegate->getNumInputs();
}

IInput* PipelinedModule::getInput(size_t i) {
	return delegate->getInput(i);
}

size_t PipelinedModule::getNumOutputs() const {
	return delegate->getNumOutputs();
}

IOutput* PipelinedModule::getOutput(size_t i) const {
	return delegate->getOutput(i);
}

void PipelinedModule::process(Data data) {
	if (data) {
		delegate->process(data);
	} else {
		endOfStream();
	}
}

void PipelinedModule::dispatch(Data data) {
	if (isSource()) {
		assert(data == nullptr);
		executor(MEMBER_FUNCTOR_PROCESS(delegate.get()), data);
	}
	executor(MEMBER_FUNCTOR_PROCESS(this), data);
}

bool PipelinedModule::isSource() const {
	return delegate->getNumOutputs() == 0;
}

bool PipelinedModule::isSink() const {
	return delegate->getNumOutputs() == 0;
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
