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
				assert(!res);
			}

			Test("connect");
			size_t id = sig.connect(Util::dummy);

			Test("single connection: check result");
			const int input = 100;
			auto numVal = sig.emit(input);
			auto val = sig.results();
			assert(numVal == val.size());
			assert(val.size() == 1);
			assert(val[0] == Util::dummy(input));

			val.clear();

			Test("single connection: check results");
			size_t id2 = sig.connect(dummy2);
			size_t id3 = sig.connect(Util::dummy);
			size_t id4 = sig.connect(dummy2);
			numVal = sig.emit(input);
			val = sig.results();
			assert(numVal == val.size());
			assert(val.size() == 4);
			assert(val[0] == Util::dummy(input));
			assert(val[1] == dummy2(input));
			assert(val[2] == Util::dummy(input));
			assert(val[3] == dummy2(input));

			Test("single connection: ask results again");
			auto val2 = sig.results();
			assert(numVal == val2.size());
			assert(val2.size() == 4);
			assert(val2[0] == Util::dummy(input));
			assert(val2[1] == dummy2(input));
			assert(val2[2] == Util::dummy(input));
			assert(val2[3] == dummy2(input));

			Test("disconnections");
			{
				bool res;
				res = sig.disconnect(id2);
				assert(res);

				res = sig.disconnect(id);
				assert(res);

				//disconnect again
				res = sig.disconnect(id);
				assert(!res);

				//wrong id
				res = sig.disconnect(id + 1);
				assert(!res);
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
	assert(!res);

	std::cout << std::endl;
	return 0;
}
#endif
