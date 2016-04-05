#include "pipeline.hpp"
#include "lib_utils/log.hpp"
#include <typeinfo>

#define EXECUTOR_SYNC         Signals::ExecutorSync<void(Data)>
#define EXECUTOR_ASYNC_THREAD Signals::ExecutorThread<void(Data)>
#define EXECUTOR_ASYNC_POOL   StrandedPoolModuleExecutor
#define EXECUTOR EXECUTOR_ASYNC_POOL

using namespace Modules;

namespace Pipelines {

class PipelinedInput : public IInput {
	public:
		PipelinedInput(IInput *input, ICompletionNotifier * const notify) : delegate(input), notify(notify) {}
		virtual ~PipelinedInput() noexcept(false) {}

		/* direct call: receiving nullptr stops the execution */
		virtual void process(Data data) override {
			if (data) {
				Log::msg(Log::Debug, format("Module %s: dispatch data for time %s", typeid(notify).name(), data->getTime() / (double)IClock::Rate));
				delegate->process(data);
			} else {
				Log::msg(Log::Debug, format("Module %s: notify finished.", typeid(notify).name()));
				notify->finished();
			}
		}

		virtual size_t getNumConnections() const override {
			return delegate->getNumConnections();
		}
		virtual void connect() override {
			delegate->connect();
		}

	private:
		IInput *delegate;
		ICompletionNotifier * const notify;
};

PipelinedModule::PipelinedModule(Module *module, ICompletionNotifier *notify)
	: delegate(module), localExecutor(new EXECUTOR), executor(*localExecutor), m_notify(notify) {
}

void PipelinedModule::mimicInputs() {
	auto const delegateInputs = delegate->getNumInputs();
	auto const thisInputs = inputs.size();
	if (thisInputs < delegateInputs) {
		for (size_t i = thisInputs; i < delegateInputs; ++i) {
			addInput(new PipelinedInput(delegate->getInput(i), this));
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
	if (delegate->getNumInputs() == 0) {
		return true;
	} else if (delegate->getNumInputs() == 1 && dynamic_cast<Modules::Input<DataLoose, Modules::IModule>*>(delegate->getInput(0))) {
		return true;
	} else {
		return false;
	}
}

bool PipelinedModule::isSink() const {
	return delegate->getNumOutputs() == 0;
}

void PipelinedModule::connect(IOutput *output, size_t inputIdx) {
	ConnectOutputToInput(output, getInput(inputIdx), executor);
}

void PipelinedModule::dispatch(Data data) {
	Log::msg(Log::Debug, format("Module %s: dispatch data", typeid(delegate).name()));

	if (isSource()) {
		assert(data == nullptr);
		if (getNumInputs() == 0) {
			/*first time: create a fake pin and push null to trigger execution*/
			delegate->addInput(new Input<DataLoose>(delegate.get()));
			executor(MEMBER_FUNCTOR_PROCESS(delegate->getInput(0)), data);
		} else {
			/*the source is likely processing: push null in the loop to exit and let things follow their way*/
			delegate->getInput(0)->push(data);
			return;
		}
	}

	for (size_t i = 0; i < getNumInputs(); ++i) {
		executor(MEMBER_FUNCTOR_PROCESS(getInput(i)), data);
	}
}

/* end of stream */
void PipelinedModule::finished() {
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

void Pipeline::connect(IPipelineModule *prev, size_t outputIdx, IPipelineModule *next, size_t inputIdx) {
	if (next->isSink())
		numRemainingNotifications++;
	next->connect(prev->getOutput(outputIdx), inputIdx);
}

void Pipeline::start() {
	Log::msg(Log::Info, "Pipeline: starting");
	for (auto &m : modules) {
		if (m->isSource())
			m->dispatch(nullptr);
	}
	Log::msg(Log::Info, "Pipeline: started");
}

void Pipeline::waitForCompletion() {
	Log::msg(Log::Info, "Pipeline: waiting for completion (remaning: %s)", (int)numRemainingNotifications);
	std::unique_lock<std::mutex> lock(mutex);
	while (numRemainingNotifications > 0) {
		condition.wait(lock);
	}
	Log::msg(Log::Info, "Pipeline: completed");
}

void Pipeline::exitSync() {
	Log::msg(Log::Warning, format("Pipeline: asked to exit now."));
	for (auto &m : modules) {
		if (m->isSource())
			m->dispatch(nullptr);
	}
}

void Pipeline::finished() {
	std::unique_lock<std::mutex> lock(mutex);
	assert(numRemainingNotifications > 0);
	--numRemainingNotifications;
	condition.notify_one();
}

}
