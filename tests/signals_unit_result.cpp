#include "tests.hpp"
#include "signals.hpp"

#include "internal/core/result.hpp"


namespace Tests {
	namespace Unit {
		namespace Result {
			template<typename T>
			bool test() {
				ResultQueueThreadSafe<T> result;
				auto res = result.get();

				return true;
			}

			int main(int argc, char **argv) {
				//TODO: complete this test... and create other unit tests
				Test("unit test on class Result");
				{
					bool res = test<int>();
					ASSERT(res);
				}
				{
					bool res = test<void>();
					ASSERT(res);
				}

				return 0;
			}
		}
	}
}

#ifdef UNIT
using namespace Tests;
int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");

	int res = 0;

	res = Unit::Result::main(argc, argv);
	ASSERT(!res);

	std::cout << std::endl;
	return 0;
}
#endif
