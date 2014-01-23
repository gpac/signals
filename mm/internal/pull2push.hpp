#pragma once

#include <../modules/modules.hpp>

namespace MM {

//TODO: we should be able to run this module easily in a separate thread
class Pull2Push : public Modules::Module {
public:
	Pull2Push(Modules::Module *module) : delegate(module) {
		signals.push_back(new Pin); //TODO: this super module should copy the structure from the delegate, see Reorder
		Connect(getSignal(0), delegate.get(), &Modules::Module::process); //delegate output to this output ; faster is delegate output signal is sync
	}
	~Pull2Push() {
		delete signals[0];
	}
	bool process(std::shared_ptr<Data> data) {
		for (;;) {
			std::shared_ptr<Data> out(signals[0]->getBuffer(0));
			signals[0]->emit(out);
			auto res = getSignal(0).results(false, true);
			if (res->size() && ((*res)[0] == false)) {
				break;
			}
		}
		return true;
	}
	bool handles(const std::string &url) {
		return delegate->handles(url);
	}

private:
	std::unique_ptr<Modules::Module> delegate;
};

}
