#pragma once

#include <../modules/modules.hpp>

namespace MM {

//TODO: we should be able to run this module easily in a separate thread
class Pull2Push : public Modules::Module {
public:
	Pull2Push(Modules::ModuleSync *module) : delegate(module) { //TODO: look if you can force them synchronous? put a caller at each call?
		signals.push_back(new Pin); //TODO: this super module should copy the structure from the delegate, see Reorder
		Connect(delegate->getPin(0)->getSignal(), this, &Pull2Push::reemit); //delegate output to this output ; faster is delegate output signal is sync
	}
	~Pull2Push() {
		delete signals[0];
	}
	bool process(std::shared_ptr<Data> data) {
#if 0
		for (;;) {
			std::shared_ptr<Data> out(signals[0]->getBuffer(0));
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
	void waitForCompletion() {
		delegate->destroy();
		destroy();
	}
	bool handles(const std::string &url) {
		return delegate->handles(url);
	}

private:
	bool reemit(std::shared_ptr<Data> data) { //output pin forwarding
		//auto res = delegate->getPin(0)->getSignal().results(false); //delegate may be async, so we must flush
		//std::cout << "Pull2Push::reemit (res->size() == " << res->size() << ")" << std::endl;
		getPin(0)->getSignal().emit(data);
		return true;
	}

	std::unique_ptr<Modules::ModuleSync> delegate;
};

}
