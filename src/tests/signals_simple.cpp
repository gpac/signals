#include "tests.hpp"
#include "lib_signals/signals.hpp"

using namespace Tests;
using namespace Signals;

namespace {
int dummy2(int a) {
	return Util::dummy(1 + Util::dummy(a));
}

unittest("signals_simple") {
	Signal<int(int)> sig;

	Test("disconnect non existing");
	{
		bool res;
		res = sig.disconnect(0);
		ASSERT(!res);
	}

	Test("connect");
	size_t id = sig.connect(Util::dummy);

	Test("single connection: check result");
	const int input = 100;
	auto numVal = sig.emit(input);
	auto val = sig.results();
	ASSERT(numVal == val->size());
	ASSERT(val->size() == 1);
	ASSERT((*val)[0] == Util::dummy(input));

	Test("multiple connections: check results");
	size_t id2 = sig.connect(dummy2);
	sig.connect(Util::dummy);
	sig.connect(dummy2);
	numVal = sig.emit(input);
	val = sig.results();
	ASSERT(numVal == val->size());
	ASSERT(val->size() == 4);
	ASSERT((*val)[0] == Util::dummy(input));
	ASSERT((*val)[1] == dummy2(input));
	ASSERT((*val)[2] == Util::dummy(input));
	ASSERT((*val)[3] == dummy2(input));

	Test("multiple connections: ask results again");
	auto val2 = sig.results();
	ASSERT(numVal == val2->size());
	ASSERT(val2->size() == 4);
	ASSERT((*val2)[0] == Util::dummy(input));
	ASSERT((*val2)[1] == dummy2(input));
	ASSERT((*val2)[2] == Util::dummy(input));
	ASSERT((*val2)[3] == dummy2(input));

	Test("disconnections");
	{
		bool res;
		res = sig.disconnect(id2);
		ASSERT(res);

		res = sig.disconnect(id);
		ASSERT(res);

		//disconnect again
		res = sig.disconnect(id);
		ASSERT(!res);

		//wrong id
		res = sig.disconnect(id + 1);
		ASSERT(!res);
	}
}

unittest("connect to lambda") {
	Signal<int(int)> sig;
	Connect(sig, [](int val) -> int { return val * val; });
	sig.emit(8);
	auto const res = sig.results();
	ASSERT(res->size() == 1);
	ASSERT((*res)[0] == 64);
}
}

