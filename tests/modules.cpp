#include "tests.hpp"
#include "modules.hpp"


#include "modules_demux.cpp"

using namespace Tests;


int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");

	int res = 0;

	res = Modules::Demux::main(argc, argv);
	ASSERT(!res);

	return 0;
}
