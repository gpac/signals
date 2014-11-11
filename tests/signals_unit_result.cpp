#include "tests.hpp"
#include "signals.hpp"
#include "internal/core/result.hpp"

using namespace Tests;
using namespace Signals;

namespace {
template<typename T>
bool test() {
	ResultQueue<T> result;
	auto res = result.get();

	return true;
}

//TODO: complete this test... and create other unit tests
unittest("unit test on class Result") {
	{
		bool res = test<int>();
		ASSERT(res);
	}
	{
		bool res = test<void>();
		ASSERT(res);
	}
}
}

