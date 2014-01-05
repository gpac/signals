#include "signals.hpp"
#include "tests.hpp"

#include <sstream>
#include <vector>


namespace Tests {
	namespace Async {
		int sleepAndDummy(int ms, int a) {
			Util::sleepInMs(ms);
			return a;
		}

		int main(int argc, char **argv) {
			Test("destroy on execution");
			{
				Signal<void(int)> sig;
				sig.connect(Util::sleepInMs);
				sig.emit(1000);
			}

			Test("disconnect before execution");
			{
				Signal<void(int)> sig;
				size_t uid = sig.connect(Util::sleepInMs);
				sig.disconnect(uid);
				sig.emit(1000);
			}

			Test("disconnect on execution");
			{
				Signal<void(int)> sig;
				size_t uid = sig.connect(Util::sleepInMs);
				sig.emit(1000);
				sig.disconnect(uid);
			}

			Test("asks results after disconnection");
			{
				Signal<int(int)> sig;
				size_t uid = sig.connect(Util::compute);
				sig.emit(27);
				sig.disconnect(uid);
				auto res = sig.results();
				ASSERT(res.size() == 0);
			}

			Test("as many results as emit() calls");
			{
				Signal<int(int)> sig;
				size_t uid = sig.connect(Util::dummy);
				sig.emit(27);
				sig.emit(1789);
				auto res = sig.results();
				ASSERT(res.size() == 2);
				ASSERT(res[0] == 27);
				ASSERT(res[1] == 1789);
			}

			Test("as many results as emit() calls, results arriving in wrong order");
			{
				Signal<int(int, int)> sig;
				size_t uid = sig.connect(sleepAndDummy);
				sig.emit(200, 27);
				sig.emit(20, 1789);
				auto res = sig.results();
				ASSERT(res.size() == 2);
				ASSERT(res[0] == 27);
				ASSERT(res[1] == 1789);
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

	res = Async::main(argc, argv);
	ASSERT(!res);

	std::cout << std::endl;
	return 0;
}
#endif
