#include "tests.hpp"
#include "lib_signals/signals.hpp"

using namespace Tests;
using namespace Signals;

namespace {

#ifdef ENABLE_FAILING_TESTS
unittest("thread-safe queue with non-pointer types") {
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

unittest("thread-safe queue can be cleared while a blocking pop() is waiting") {
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

unittest("thread-safe queue can be cleared while several blocking pop() are waiting") {
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

unittest("thread-safe queue has an optional max size") {
	const int maxSize = 2;
	QueueMaxSize<int> queue(maxSize);
	bool res;
	for (int i = 0; i < maxSize; ++i) {
		res = queue.tryPush(i);
		ASSERT_EQUALS(true, res);
	}
	res = queue.tryPush(maxSize);
	ASSERT_EQUALS(false, res);

	int val;
	res = queue.tryPop(val);
	ASSERT_EQUALS(true, res);
	ASSERT_EQUALS(0, val);
	queue.clear();
}

unittest("thread-safe queue has an optional blocking max size") {
	const int maxSize = 1;
	QueueMaxSize<int> queue(maxSize);
	std::condition_variable mustPop;

	auto f2 = [&]() {
		for (int i = 0; i < maxSize; ++i) {
			queue.push(i);
		}
		bool res = queue.tryPush(maxSize);
		ASSERT_EQUALS(false, res);
		auto f1 = [&]() {
			queue.push(maxSize); //blocking
		};
		std::thread tf1(f1);
		mustPop.notify_one();
		tf1.join();
	};

	std::thread tf2(f2);
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);
	mustPop.wait(lock);
	int val;
	for (int i = 0; i <= maxSize; ++i) {
		val = queue.pop();
		ASSERT_EQUALS(i, val);
	}
	bool res = queue.tryPop(val);
	ASSERT_EQUALS(false, res);
	queue.clear();
	tf2.join();
}

unittest("thread-safe queue can be cleared with several blocking push() calls") {
	const int maxSize = 1;
	ASSERT(maxSize < 4); //the example has 4 threads
	QueueMaxSize<int> queue(maxSize);

	auto f = [&]() {
		queue.push(0);
	};

	std::thread tf1(f);
	std::thread tf2(f);
	std::thread tf3(f);
	std::thread tf4(f);
	queue.clear();
	tf1.join();
	queue.clear();
	tf2.join();
	queue.clear();
	tf3.join();
	queue.clear();
	tf4.join();
}

unittest("thread-safe queue can be destroyed while element is blocked while pushing") {
	const int maxSize = 1;
	QueueMaxSize<int> queue(maxSize);

	auto f = [&]() {
		for (int i = 0; i < maxSize; ++i) {
			queue.push(i);
		}
		queue.push(maxSize); //blocking
	};
	std::thread tf(f);
	queue.clear();
	tf.join();
}

}

