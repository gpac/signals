#include "signal.hpp"
#include "tests.hpp"

#include <iostream>


#include "signals_simple.cpp"
#include "signals_perf.cpp"

using namespace Tests;


int main(int argc, char **argv) {
	int res;
	
	res = Simple::main(argc, argv);
	assert(!res);

	res = Perf::main(argc, argv);
	assert(!res);

	return 0;
}
