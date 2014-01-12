#include "tests.hpp"
#include "modules.hpp"


#include "modules_simple.cpp"
#include "modules_demux.cpp"

using namespace Tests;


int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");
	Tests::RunAll();
	return 0;
}
