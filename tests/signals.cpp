#include "signal.hpp"
#include "tests.hpp"

#include <iostream>


#include "signals_simple.cpp"
#include "signals_perf.cpp"

using namespace Tests;


int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");

	int res = 0;
	
	res = Simple::main(argc, argv);
	assert(!res);

	res = Perf::main(argc, argv);
	assert(!res);

	std::cout << std::endl;
	return 0;
}
