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
			Test("basic module connection tests");
			{
				Signaler sender;
				Signaler &senderRef = sender;
				Signaler *senderPtr = &sender;
				Slot receiver;
				Slot &receiverRef = receiver;
				Slot *receiverPtr = &receiver;
				CONNECT(&sender, signal, &receiver, &Slot::slot);
				CONNECT(&senderRef, signal, &receiver, &Slot::slot);
				CONNECT(senderPtr, signal, &receiver, &Slot::slot);
				CONNECT(senderPtr, signal, &receiverRef, &Slot::slot);
				CONNECT(senderPtr, signal, receiverPtr, &Slot::slot);

				sender.signal.emit(100);
				auto res = sender.signal.results();
				ASSERT(res->size() == 5);
#ifdef ENABLE_FAILING_TESTS
				ASSERT(res[0] == 101 && res[4] == 101);
#endif
			}

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

