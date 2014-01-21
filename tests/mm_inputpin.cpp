#include "tests.hpp"
#include <mm.hpp>
#include "../modules/modules.hpp"
#include <memory>
#include <utility>

using namespace Tests;
using namespace MM;

namespace {

	class Osc : public Modules::ModuleSync {
	public:
		Osc() : seqNumber(0) {
			signals.push_back(new Pin<>);
		}
		bool process(std::shared_ptr<Data> /*sample*/) {
			seqNumber = (seqNumber + 1) % 256;
			std::shared_ptr<Data> out(signals[0]->getBuffer(32));
			out->data()[0] = seqNumber;
			signals[0]->signal.emit(out);
			return true;
		}
		bool handles(const std::string &url) {
			return false;
		}

	private:
		int seqNumber;
	};

	class Sink {
	public:
		bool process(std::shared_ptr<Data> sample) {
			return true;
		}
		bool handles(const std::string &url) {
			return false;
		}
	};

	class Amp : public Modules::ModuleSync { //making it sync for perf (avoid spawning one more time when forwarding the output pin)
	public:
		Amp() : seqNumber(0) {
			signals.push_back(new Pin<>);
		}
		bool handles(const std::string &url) {
			return false;
		}
		bool process(std::shared_ptr<Data> sample) {
			int idx = sample->data()[0];
			{
				// check packet order
				int expectedIdx = (seqNumber + 1) % 256;
				if (idx != expectedIdx) {
					std::cout << "Amp ERROR: expected " << expectedIdx << " received " << idx << std::endl;
				}
			}
			seqNumber = idx;
			signals[0]->signal.emit(sample);
			return true;
		}
		void waitForCompletion() { //unnecessary - just in case you want to try without the AmpReordered
			destroy();
		}

	private:
		int seqNumber;
	};

	/**
	 * A class which gets a copy from the last result. We don't want a shared_ptr to result in this case,
	 * because emit() (which reset results) an results() are called in different threads. Thus would
	 * require an external lock to protect the result.
	 */
	template<typename ResultType>
	class ResultLast : public IResult {
	public:
		typedef ResultType ResultValue;
		explicit ResultLast() {
		}
		void set(ResultType r) {
			last = r;
		}
		ResultValue& get() {
			return last;
		}
		void clear() {
		}

	private:
		ResultType last;
	};

	/**
	 * A module, containing a delegate module.
	 * This is the same principle as the current MM:Module preprocessor, but the relation between the input pin
	 *  and the delegate is async but guarantees the packet order.
	 * Therefore you should connect to this module synchronously.
	 */
	class AmpReordered : public Modules::Module {
	public:
		AmpReordered(Modules::ModuleSync *module) : delegate(module) {
			signals.push_back(new Pin<>); //this super module should copy the structure from the delegate
			Connect(synchronizerSignal, this, &AmpReordered::reflector);
			//TODO: connect to lambdas: Connect(synchronizerSignal, this, [](std::shared_ptr<Data> sample)->std::shared_ptr<Data> { return sample; });
			Connect(internalSignal, this, &AmpReordered::processInOrder);
			Connect(delegate->signals[0]->signal, this, &AmpReordered::reemit); //delegate output to this output ; faster is delegate output signal is sync
		}
		void waitForCompletion() {
			delegate->destroy();
			destroy();
		}
		bool handles(const std::string &url) {
			return false;
		}
		bool process(std::shared_ptr<Data> sample) {
			synchronizerSignal.emit(sample);
			internalSignal.emit();
			return true;
		}

	private:
		bool processInOrder() {
			auto res = synchronizerSignal.results(true, true); //wait synchronously for the next
			return delegate->process(res);
		}
		bool reemit(std::shared_ptr<Data> data) { //output pin forwarding
			signals[0]->signal.emit(data);
			return true;
		}
		std::shared_ptr<Data> reflector(std::shared_ptr<Data> sample) {
			return sample;
		}
		std::unique_ptr<Modules::ModuleSync> delegate;
		Signal<std::shared_ptr<Data>(std::shared_ptr<Data>), ResultLast<std::shared_ptr<Data>>> synchronizerSignal;
		Signal<bool(void), ResultQueueThreadSafe<bool>, CallerThread> internalSignal;
	};

	unittest("Simple synth") {
		std::unique_ptr<Osc> osc(new Osc);
		std::unique_ptr<AmpReordered> amp1(new AmpReordered(new Amp));
		std::unique_ptr<AmpReordered> amp2(new AmpReordered(new Amp));
		std::unique_ptr<Sink> sink1(new Sink);
		std::unique_ptr<Sink> sink2(new Sink);

		Connect(osc->signals[0]->signal, amp1.get(), &AmpReordered::process);
		Connect(osc->signals[0]->signal, amp2.get(), &AmpReordered::process);
		Connect(amp1->signals[0]->signal, sink1.get(), &Sink::process);
		Connect(amp2->signals[0]->signal, sink2.get(), &Sink::process);

		for (int i = 0; i < 10000; ++i) { //this is bigger than the default allocator size (100), so they will be contention
			osc->process(nullptr);
		}

		//osc->destroy(); //not mandatory since later filters (Amp and Sink) use data from the same allocator
		amp1->waitForCompletion();
		amp2->waitForCompletion();
	}

}

