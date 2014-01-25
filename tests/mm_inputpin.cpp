#include "tests.hpp"
#include <mm.hpp>
#include "../modules/modules.hpp"
#include <memory>
#include <utility>

using namespace Tests;
using namespace MM;

namespace {

	class Osc : public Modules::Module {
	public:
		Osc() : seqNumber(0) {
			signals.push_back(pinFactory->createPin());
		}
		bool process(std::shared_ptr<Data> /*sample*/) {
			seqNumber = (seqNumber + 1) % 256;
			std::shared_ptr<Data> out(signals[0]->getBuffer(32));
			out->data()[0] = seqNumber;
			getPin(0)->getSignal().emit(out);
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

	class Amp : public Modules::Module { //making it sync for perf (avoid spawning one more time when forwarding the output pin)
	public:
		Amp() : seqNumber(0) {
			signals.push_back(pinFactory->createPin());
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
			getPin(0)->getSignal().emit(sample);
			return true;
		}
		void waitForCompletion() { //unnecessary - just in case you want to try without the AmpReordered
			Modules::Module::waitForCompletion();
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

		Connect(osc->getPin(0)->getSignal(), amp1.get(), &Reorder::process);
		Connect(osc->getPin(0)->getSignal(), amp2.get(), &Reorder::process);
		Connect(amp1->getPin(0)->getSignal(), sink1.get(), &Sink::process);
		Connect(amp2->getPin(0)->getSignal(), sink2.get(), &Sink::process);

		for (int i = 0; i < 10000; ++i) { //this is bigger than the default allocator size (100), so they will be contention
			osc->process(nullptr);
		}

		//osc->waitForCompletion(); //not mandatory since later filters (Amp and Sink) use data from the same allocator
		amp1->waitForCompletion();
		amp2->waitForCompletion();
	}

}

