#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include "../utils/tools.hpp"
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

void Run(int i) {
	if(i < 0 || i >= g_NumTests)
		throw std::runtime_error("Invalid test index");
	std::cout << "----------------------------------------------------------------" << std::endl;
	std::cout << "Test #" << i << ": " << g_AllTests[i].name << std::endl;
	g_AllTests[i].fn();
	std::cout << std::endl;
}

void RunAll() {
	for(int i=0; i < g_NumTests; ++i) {
		Run(i);
	}
}

void listAll() {
	for(int i=0; i < g_NumTests; ++i) {
		std::cout << "Test #" << i << ": " << g_AllTests[i].name << std::endl;
	}
}
}

int main(int argc, const char* argv[]) {
	Tools::Profiler p("TESTS TOTAL TIME");
	if(argc == 1)
		Tests::RunAll();
	else if(argc == 2) {
		auto const word = std::string(argv[1]);
		if(word == "--list" || word == "-l") {
			Tests::listAll();
		} else {
			int idx = atoi(argv[1]);
			Tests::Run(idx);
		}
	}
	return 0;
}
