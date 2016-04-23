#pragma once

#include "../core/module.hpp"
#include <memory>
#include <vector>


namespace Pipelines {

struct IPipelineModule : public Modules::IProcessor, public Modules::IInputCap, public Modules::IOutputCap {
	virtual ~IPipelineModule() noexcept(false) {}
};

struct IPipelinedModule : public IPipelineModule {
	virtual bool isSource() const = 0;
	virtual bool isSink() const = 0;
	virtual void connect(Modules::IOutput *output, size_t inputIdx) = 0;
};

struct ICompletionNotifier {
	virtual void finished() = 0;
};

class Pipeline : public ICompletionNotifier {
	public:
		Pipeline(bool isLowLatency = false);
		IPipelineModule* addModule(Modules::Module* rawModule);
		void connect(IPipelineModule *prev, size_t outputIdx, IPipelineModule *next, size_t inputIdx);
		void start();
		void waitForCompletion();
		void exitSync(); /*ask for all sources to finish*/

	private:
		void finished() override;

		std::vector<std::unique_ptr<IPipelinedModule>> modules;
		bool isLowLatency;

		std::mutex mutex;
		std::condition_variable condition;
		std::atomic<int> numRemainingNotifications;
};

}
