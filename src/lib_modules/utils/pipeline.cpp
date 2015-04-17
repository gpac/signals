#include "modules.hpp"
#include "pipeline.hpp"


#define EXECUTOR_SYNC ExecutorSync<void(std::shared_ptr<const Data>)>
#define EXECUTOR_ASYNC StrandedPoolModuleExecutor
#define EXECUTOR EXECUTOR_ASYNC


namespace Modules {

/* take ownership of module */
PipelinedModule::PipelinedModule(IModule *module, ICompletionNotifier *notify)
: type(None), delegate(module), localExecutor(new EXECUTOR), executor(*localExecutor), m_notify(notify) {
}

void PipelinedModule::connect(IOutput* out) {
	ConnectToModule(out->getSignal(), this, executor);
}

size_t PipelinedModule::getNumOutputs() const {
  return delegate->getNumOutputs();
}

IOutput* PipelinedModule::getOutput(size_t i) const {
  return delegate->getOutput(i);
}

/* direct call: receiving nullptr stops the execution */
void PipelinedModule::process(std::shared_ptr<const Data> data) {
	if (data) {
		delegate->process(data);
	} else {
		endOfStream();
	}
}

/* same as process() but uses the executor (may defer the call) */
void PipelinedModule::dispatch(std::shared_ptr<const Data> data) {
	if (isSource()) {
		assert(data == nullptr);
		executor(MEMBER_FUNCTOR(delegate.get(), &ModuleS::process), data);
	}
	executor(MEMBER_FUNCTOR(this, &PipelinedModule::process), data);
}

/* source modules are stopped manually - then the message propagates to other connected modules */
void PipelinedModule::setSource(bool isSource) {
	type = isSource ? Source : None;
}

bool PipelinedModule::isSource() const {
	return type == Source;
}

bool PipelinedModule::isSink() const {
	return delegate->getNumOutputs() == 0;
}

void PipelinedModule::endOfStream() {
	delegate->flush();
	if (isSink()) {
		m_notify->finished();
	} else {
		for (size_t i = 0; i < delegate->getNumOutputs(); ++i) {
			delegate->getOutput(i)->emit(std::shared_ptr<const Data>(nullptr));
		}
	}
}

Pipeline::Pipeline(bool isLowLatency) : isLowLatency(isLowLatency), numRemainingNotifications(0) {
}

PipelinedModule* Pipeline::addModule(IModule* rawModule, bool isSource) {
	if(!rawModule)
		return nullptr;
	rawModule->setLowLatency(isLowLatency);
	auto module = uptr(new PipelinedModule(rawModule, this));
	auto ret = module.get();
	module->setSource(isSource);
	modules.push_back(std::move(module));
	return ret;
}

void Pipeline::connect(IOutput* out, PipelinedModule *module) {
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

