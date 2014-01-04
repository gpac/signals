#include "signal.hpp"
#include "tests.hpp"


namespace Tests {
	namespace Module {
		class Signaller {
		public:
			Signal<int(int)> signal;
		};

		class Slot {
		public:
			int slot(int a) {
				std::cout << "a=" << a << std::endl;
				return a+1;
			}
		};

		int main(int argc, char **argv) {
			Signaller sender;
			Slot receiver;
			std::function<int(int)> f = std::bind(&Slot::slot, receiver, std::placeholders::_1);
			sender.signal.connect(f);
			sender.signal.emit(100);
			sender.signal.results();

			Signaller sender2;
			Slot receiver2;
			sender.signal.emit(100);
			return 0;
		}
	}
}

#ifdef UNIT
using namespace Tests;
int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");

	int res = 0;

	res = Module::main(argc, argv);
	assert(!res);

	std::cout << std::endl;
	return 0;
}
#endif

