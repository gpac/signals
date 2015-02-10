#include "tests.hpp"
#include "lib_signals/signals.hpp"
#include <sstream>
#include <vector>

using namespace Tests;
using namespace Signals;

namespace {

int sleepAndDummy(int ms, int a) {
	Util::sleepInMs(ms);
	return a;
}

unittest("destroy on execution") {
	Signal<void(int)> sig;
	sig.connect(Util::sleepInMs);
	sig.emit(1000);
}

unittest("disconnect before execution") {
	Signal<void(int)> sig;
	size_t uid = sig.connect(Util::sleepInMs);
	sig.disconnect(uid);
	sig.emit(1000);
}

unittest("disconnect on execution") {
	Signal<void(int)> sig;
	size_t uid = sig.connect(Util::sleepInMs);
	sig.emit(1000);
	sig.disconnect(uid);
}

unittest("as many results as emit() calls") {
	Signal<int(int)> sig;
	sig.connect(Util::dummy);
	sig.emit(27);
	sig.emit(1789);
	auto res = sig.results();
	ASSERT(res->size() == 2);
	ASSERT((*res)[0] == 27);
	ASSERT((*res)[1] == 1789);
}

unittest("as many results as emit() calls, results arriving in wrong order") {
	Signal<int(int, int)> sig;
	sig.connect(sleepAndDummy);
	sig.emit(200, 27);
	sig.emit(20, 1789);
	auto res = sig.results();
	ASSERT(res->size() == 2);
	ASSERT((*res)[0] == 27);
	ASSERT((*res)[1] == 1789);
}

unittest("as many results as emit() calls, results arriving in wrong order") {
	Signal<int(int, int)> sig;
	sig.connect(sleepAndDummy);
	sig.emit(200, 27);
	sig.emit(20, 1789);
	auto res = sig.results();
	ASSERT(res->size() == 2);
	ASSERT((*res)[0] == 27);
	ASSERT((*res)[1] == 1789);
}
}

