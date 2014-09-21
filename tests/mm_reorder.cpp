#include "tests.hpp"
#include <mm.hpp>
#include "../modules/modules.hpp"
#include <utility>

using namespace Tests;
using namespace MM;

namespace {

class Osc : public Modules::Module {
public:
	Osc() : seqNumber(0) {
		signals.push_back(uptr(pinFactory->createPin()));
	}
	bool process(std::shared_ptr<Data> /*sample*/) {
		seqNumber = (seqNumber + 1) % 256;
		auto out(signals[0]->getBuffer(32));
		out->data()[0] = seqNumber;
		getPin(0)->getSignal().emit(out);
		return true;
	}

private:
	int seqNumber;
};

class Sink {
public:
	bool process(std::shared_ptr<Data> sample) {
		return true;
	}
};

class Amp : public Modules::Module { //making it sync for perf (avoid spawning one more time when forwarding the output pin)
public:
	Amp() : seqNumber(0) {
		signals.push_back(uptr(pinFactory->createPin()));
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

unittest("Simple synth Romain") {
	auto osc = uptr(new Osc);
	auto amp1 = uptr(new Amp);
	auto amp2 = uptr(new Amp);
	auto sink1 = uptr(new Sink);
	auto sink2 = uptr(new Sink);

	ExecutorThread<bool(std::shared_ptr<Data >)> amp1Thread, amp2Thread, sink1Thread, sink2Thread;

	Connect(osc->getPin(0)->getSignal(), amp1.get(), &Amp::process, amp1Thread);
	Connect(osc->getPin(0)->getSignal(), amp2.get(), &Amp::process, amp2Thread);
	Connect(amp1->getPin(0)->getSignal(), sink1.get(), &Sink::process, sink1Thread);
	Connect(amp2->getPin(0)->getSignal(), sink2.get(), &Sink::process, sink2Thread);

	for (int i = 0; i < 10000; ++i) { //this is bigger than the default allocator size (100), so there will be contention
		osc->process(nullptr);
	}

	//osc->waitForCompletion(); //not mandatory since later filters (Amp and Sink) use data from the same allocator
	amp1->waitForCompletion();
	amp2->waitForCompletion();
}

}

