#include "signal.hpp"
#include "tests.hpp"


namespace Tests {
	namespace Simple {
		int dummy2(int a) {
			return Util::dummy(Util::dummy(a));
		}

		int main(int argc, char **argv) {
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
			ASSERT(numVal == val.size());
			ASSERT(val.size() == 1);
			ASSERT(val[0] == Util::dummy(input));

			Test("multiple connections: check results");
			size_t id2 = sig.connect(dummy2);
			size_t id3 = sig.connect(Util::dummy);
			size_t id4 = sig.connect(dummy2);
			numVal = sig.emit(input);
			val = sig.results();
			ASSERT(numVal == val.size());
			ASSERT(val.size() == 4);
			ASSERT(val[0] == Util::dummy(input));
			ASSERT(val[1] == dummy2(input));
			ASSERT(val[2] == Util::dummy(input));
			ASSERT(val[3] == dummy2(input));

			Test("multiple connections: ask results again");
			auto val2 = sig.results();
			ASSERT(numVal == val2.size());
			ASSERT(val2.size() == 4);
			ASSERT(val2[0] == Util::dummy(input));
			ASSERT(val2[1] == dummy2(input));
			ASSERT(val2[2] == Util::dummy(input));
			ASSERT(val2[3] == dummy2(input));

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

			return 0;
		}
	}
}

#ifdef UNIT
using namespace Tests;
int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");

	int res = 0;

	res = Simple::main(argc, argv);
	ASSERT(!res);

	std::cout << std::endl;
	return 0;
}
#endif
