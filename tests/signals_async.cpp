#include "signal.hpp"
#include "tests.hpp"

#include <sstream>
#include <vector>


namespace Tests {
	namespace Async {
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
				assert(res.size() == 0);
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
	assert(!res);

	std::cout << std::endl;
	return 0;
}
#endif
