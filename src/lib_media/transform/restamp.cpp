#include "restamp.hpp"

namespace Modules {
	namespace Transform {

		Restamp::Restamp(Mode mode, int64_t offsetIn180k)
		: offset(offsetIn180k), mode(mode) {
			addInput(new Input<DataBase>(this));
			addOutput<OutputDefault>();
		}

		Restamp::~Restamp() {
		}

		void Restamp::process(Data data) {
			uint64_t time;
			switch (mode) {
			case Passthru:
				time = data->getTime();
				break;
			case Reset:
				time = data->getTime();
				if (!isInitTime) {
					isInitTime = true;
					offset -= time;
				}
				break;
			case ClockSystem:
				time = g_DefaultClock->now();
				if (!isInitTime) {
					isInitTime = true;
					offset -= time;
				}
				break;
			default:
				throw error("Unknown mode");
			}

			log(Debug, "%s -> %sms", (double)data->getTime() / IClock::Rate, (double)(time + offset) / IClock::Rate);
			const_cast<DataBase*>(data.get())->setTime(time + offset); //FIXME: we should have input&output on the same allocator
			getOutput(0)->emit(data);
		}

	}
}
