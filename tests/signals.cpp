#include "tests.hpp"
#include "signals.hpp"
#include <iostream>


//#define ENABLE_FAILING_TESTS


#include "signals_unit_result.cpp"
#include "signals_simple.cpp"
#include "signals_module.cpp"
#include "signals_async.cpp"
#include "signals_perf.cpp"

using namespace Tests;


int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");
	Tests::RunAll();
	return 0;
}
