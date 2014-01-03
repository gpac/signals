#include "signal.hpp"

#include <iostream>

namespace Tests {
#include "signals_simple.cpp"
#include "signals_perf.cpp"
}

using namespace Tests;

int main(int argc, char **argv) {
	int res;
	res = Simple::main(argc, argv);
	//res = Perf::main(argc, argv);
	return 0;
}
