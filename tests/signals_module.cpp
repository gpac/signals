#include "tests.hpp"
#include "signals.hpp"

using namespace Tests;
using namespace Signals;

namespace {
class Signaler {
public:
	Signal<int(int)> signal;
};

class Slot {
public:
	int slot(int a) {
		return 1 + Util::dummyPrint(a);
	}
};

unittest("basic module connection tests") {
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
	ASSERT((*res)[0] == 101 && (*res)[4] == 101);
}
}

