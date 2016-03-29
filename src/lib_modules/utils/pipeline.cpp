#include "pipeline.hpp"

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
				delegate->process(data);
			} else {
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
		for (size_t i = thisInputs; i < delegateInputs; ++i)
			addInput(new PipelinedInput(delegate->getInput(i), this));
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

void PipelinedModule::connect(IOutput *output, size_t inputIdx) {
	ConnectOutputToInput(output, getInput(inputIdx), executor);
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
