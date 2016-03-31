#include "restamp.hpp"

namespace Modules {
	namespace Transform {

		Restamp::Restamp(Mode mode, int64_t offsetIn180k)
		: offset(offsetIn180k), mode(mode) {
			addInput(new Input<DataBase>(this));
			addOutput(new OutputDefault);
		}

		Restamp::~Restamp() {
		}

		void Restamp::ensureInit(uint64_t time) {
			if (mode == Reset) {
				if (initTime == -1) {
					initTime = time;
					offset -= initTime;
				}
			}
		}

		void Restamp::process(Data data) {
			auto time = data->getTime();
			ensureInit(time);
			const_cast<DataBase*>(data.get())->setTime(time + offset); //FIXME: we should have input&output on the same allocator
			getOutput(0)->emit(data);
		}

		int64_t Restamp::getOffset() const {
			return offset;
		}

	}
}
