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
			signals.push_back(new Pin);
		}
		bool process(std::shared_ptr<Data> /*sample*/) {
			seqNumber = (seqNumber + 1) % 256;
			std::shared_ptr<Data> out(signals[0]->getBuffer(32));
			out->data()[0] = seqNumber;
			getSignal(0).emit(out);
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
			signals.push_back(new Pin);
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
			getSignal(0).emit(sample);
			return true;
		}
		void waitForCompletion() { //unnecessary - just in case you want to try without the AmpReordered
			destroy();
		}

	private:
		int seqNumber;
	};

	unittest("Simple synth") {
		std::unique_ptr<Osc> osc(new Osc);
		std::unique_ptr<Reorder> amp1(new Reorder(new Amp));
		std::unique_ptr<Reorder> amp2(new Reorder(new Amp));
		std::unique_ptr<Sink> sink1(new Sink);
		std::unique_ptr<Sink> sink2(new Sink);

		Connect(osc->getSignal(0), amp1.get(), &Reorder::process);
		Connect(osc->getSignal(0), amp2.get(), &Reorder::process);
		Connect(amp1->getSignal(0), sink1.get(), &Sink::process);
		Connect(amp2->getSignal(0), sink2.get(), &Sink::process);

		for (int i = 0; i < 10000; ++i) { //this is bigger than the default allocator size (100), so they will be contention
			osc->process(nullptr);
		}

		//osc->destroy(); //not mandatory since later filters (Amp and Sink) use data from the same allocator
		amp1->waitForCompletion();
		amp2->waitForCompletion();
	}

}

