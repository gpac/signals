#include <iostream>
#include "tests.hpp"

namespace {
typedef void (*TestFunction)();

struct UnitTest {
	void (*fn)();
	const char* name;
};

UnitTest g_AllTests[65536];
int g_NumTests;
}

namespace Tests {

int RegisterTest(void (*fn)(), const char* testName, int&) {
	g_AllTests[g_NumTests].fn = fn;
	g_AllTests[g_NumTests].name = testName;
	++g_NumTests;
	return 0;
}

void RunAll() {
	for(int i=0; i < g_NumTests; ++i) {
		std::cout << "[ ***** " << g_AllTests[i].name << " ***** ]" << std::endl;
		g_AllTests[i].fn();
		std::cout << std::endl;
	}
}
}

int main(int argc, char **argv) {
	std::cout << "Romain\n" << std::endl;
	Tests::Util::Profiler p("TESTS TOTAL TIME");
	Tests::RunAll();
	return 0;
}
