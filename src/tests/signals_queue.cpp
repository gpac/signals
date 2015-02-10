#include "tests.hpp"
#include "signals.hpp"

using namespace Tests;
using namespace Signals;

namespace {

#ifdef ENABLE_FAILING_TESTS
unittest("Thread-safe queue with non-pointer types") {
	Queue<int> queue;
	const int val = 1;

	queue.push(val);
	auto data = queue.pop();
	ASSERT(data == val);

	queue.push(val);
	data = queue.tryPop();
	ASSERT(data == val);
	data = queue.tryPop();
	ASSERT(XXX);

	queue.push(val);
	auto res = queue.tryPop(data);
	ASSERT((res == true) && (data == val));
	auto res = queue.tryPop(data);
	ASSERT(res == false);

	queue.clear();
	auto res = queue.tryPop(data);
	ASSERT(res == false);
}
#endif

unittest("Thread-safe queue can be cleared while a blocking pop() is waiting") {
	Queue<int> queue;
	auto f = [&]() {
		auto data = queue.pop();
		ASSERT_EQUALS(7, data);
	};
	std::thread tf(f);
	queue.clear();
	queue.push(7);
	tf.join();
}

unittest("Thread-safe queue can be cleared while several blocking pop() are waiting") {
	Queue<int> queue;
	auto f = [&]() {
		auto data = queue.pop();
		ASSERT_EQUALS(9, data);
	};
	std::thread tf1(f);
	std::thread tf2(f);
	std::thread tf3(f);
	queue.clear();
	queue.push(9);
	queue.push(9);
	queue.push(9);
	tf1.join();
	tf2.join();
	tf3.join();
}
}

