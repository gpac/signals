#pragma once

#include <../modules/modules.hpp>

#include "demux/gpac_demux_mp4_simple.hpp"
#include "../utils/tools.hpp"

namespace MM {

//TODO: we should be able to run this module easily in a separate thread
class Pull2Push : public Modules::Module {
public:
	Pull2Push(Modules::Module *module) : delegate(module) { //TODO: look if you can force them synchronous? put an executor at each call?
		signals.push_back(uptr(pinFactory->createPin())); //TODO: this super module should copy the structure from the delegate
		Connect(delegate->getPin(0)->getSignal(), this, &Pull2Push::reemit); //delegate output to this output ; faster is delegate output signal is sync
	}
	bool process(std::shared_ptr<Data> data) {
#if 0
		for (;;) {
			auto out(signals[0]->getBuffer(0));
			signals[0]->emit(out);
			auto res = getPin(0)->getSignal().results(false, true);
			if (res->size() && ((*res)[0] == false)) {
				break;
			}
		}
#endif
		while (delegate->process(data)) {
		}
		return true;
	}

private:
	bool reemit(std::shared_ptr<Data> data) { //output pin forwarding
		getPin(0)->getSignal().flushAvailableResults();
		getPin(0)->getSignal().emit(data);
		return true;
	}

	std::unique_ptr<Modules::Module> delegate;
};

}
