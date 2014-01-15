#pragma once

#include "submodule.hpp"
#include <atomic>

namespace MM {

class Pull2Push : public Submodule {
public:
	~Pull2Push() {
		delete signals[0];
	}

	static Pull2Push* create() {
		return new Pull2Push();
	}

	bool process(std::shared_ptr<Data> data) {
		while (1) {
			std::shared_ptr<Data> out(signals[0]->getBuffer(0));
			signals[0]->emit(out);
			auto res = signals[0]->signal.results(false, true); //this code also works with async, why not use it?
			if (res->size() && ((*res)[0] == false)) {
				break;
			}
		}
		return true;
	}

	bool handles(const std::string &url) {
		return true;
	}

private:
	Pull2Push() {
		signals.push_back(new PinSync());
	}
};

}
