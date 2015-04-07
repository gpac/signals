#include "tests.hpp"
#include "lib_signals/signals.hpp"
#include "lib_signals/internal/core/result.hpp"

using namespace Tests;
using namespace Signals;

namespace {
template<typename T>
bool test() {
	ResultQueue<T> result;
	auto res = result.get();

	return true;
}

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

