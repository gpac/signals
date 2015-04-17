#include "modules.hpp"
#include "pipeline.hpp"


#define EXECUTOR_SYNC ExecutorSync<void(Data)>
#define EXECUTOR_ASYNC StrandedPoolModuleExecutor
#define EXECUTOR EXECUTOR_ASYNC


namespace Modules {

/* take ownership of module */
template<typename ModuleType>
PipelinedModule<ModuleType>::PipelinedModule(ModuleType *module, ICompletionNotifier *notify)
: type(None), delegate(module), localExecutor(new EXECUTOR), executor(*localExecutor), m_notify(notify) {
}

template<typename ModuleType>
void PipelinedModule<ModuleType>::connect(IOutput* out) {
	ConnectToModule(out->getSignal(), this, executor);
}

template<typename ModuleType>
size_t PipelinedModule<ModuleType>::getNumOutputs() const {
  return delegate->getNumOutputs();
}

template<typename ModuleType>
IOutput* PipelinedModule<ModuleType>::getOutput(size_t i) const {
  return delegate->getOutput(i);
}

/* direct call: receiving nullptr stops the execution */
template<typename ModuleType>
void PipelinedModule<ModuleType>::process(Data data) {
	if (data) {
		delegate->process(data);
	} else {
		endOfStream();
	}
}

/* same as process() but uses the executor (may defer the call) */
template<typename ModuleType>
void PipelinedModule<ModuleType>::dispatch(Data data) {
	if (isSource()) {
		assert(data == nullptr);
		executor(MEMBER_FUNCTOR(delegate.get(), &ModuleS::process), data);
	}
	executor(MEMBER_FUNCTOR(this, &PipelinedModule::process), data);
}

/* source modules are stopped manually - then the message propagates to other connected modules */
template<typename ModuleType>
void PipelinedModule<ModuleType>::setSource(bool isSource) {
	type = isSource ? Source : None;
}

template<typename ModuleType>
bool PipelinedModule<ModuleType>::isSource() const {
	return type == Source;
}

template<typename ModuleType>
bool PipelinedModule<ModuleType>::isSink() const {
	return delegate->getNumOutputs() == 0;
}

template<typename ModuleType>
void PipelinedModule<ModuleType>::endOfStream() {
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

template<typename ModuleType>
PipelinedModule<ModuleType>* Pipeline::addModule(ModuleType* rawModule, bool isSource) {
	if(!rawModule)
		return nullptr;
	rawModule->setLowLatency(isLowLatency);
	auto module = uptr(new PipelinedModule<ModuleType>(rawModule, this));
	auto ret = module.get();
	module->setSource(isSource);
	modules.push_back(std::move(module));
	return ret;
}

template<typename ModuleType>
void Pipeline::connect(IOutput* out, PipelinedModule<ModuleType> *module) {
	if (module->isSink())
		numRemainingNotifications++;
	module->connect(out);
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

