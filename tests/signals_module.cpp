#include "signals.hpp"
#include "tests.hpp"


namespace Tests {
	namespace Module {
		class Signaler {
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
			Signaler sender;
			Signaler &senderRef = sender;
			Signaler *senderPtr = &sender; //TODO: tester const?
			Slot receiver;
			auto f1 = MEMBER_FUNCTOR(receiver, Slot::slot);
			sender.signal.connect(f1);
			CONNECT(&sender   , signal, receiver, Slot::slot);
			CONNECT(&senderRef, signal, receiver, Slot::slot);
			CONNECT(senderPtr , signal, receiver, Slot::slot);
			//CONNECT(this, ...)

			sender.signal.emit(100);
			auto res = sender.signal.results();
			ASSERT(res.size() == 1);
			ASSERT(res[0] == 101);

			Test("module connection XXX");
			Signaler sender2;
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

