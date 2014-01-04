#include "signal.hpp"
#include "tests.hpp"

#include <iostream>


//#define ENABLE_FAILING_TESTS


#include "signals_simple.cpp"
#include "signals_module.cpp"
#include "signals_async.cpp"
#include "signals_perf.cpp"

using namespace Tests;


int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");

	int res = 0;

	res = Simple::main(argc, argv);
	ASSERT(!res);

	res = Module::main(argc, argv);
	ASSERT(!res);

	res = Async::main(argc, argv);
	ASSERT(!res);

	res = Perf::main(argc, argv);
	ASSERT(!res);

	std::cout << std::endl;
	return 0;
}
