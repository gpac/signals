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
				return 1+Util::dummyPrint(a);
			}
		};

		int main(int argc, char **argv) {
			Test("module connection");
			Signaller sender;
			Slot receiver;
			std::function<int(int)> f = std::bind(&Slot::slot, receiver, std::placeholders::_1);
			sender.signal.connect(f);
			sender.signal.emit(100);
			sender.signal.results();

			Test("module connection XXX");
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
	ASSERT(!res);

	std::cout << std::endl;
	return 0;
}
#endif

