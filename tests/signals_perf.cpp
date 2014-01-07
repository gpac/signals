#include "signals.hpp"
#include "tests.hpp"

#include <sstream>
#include <vector>


namespace Tests {
	namespace Perf {
		template<typename SignalSignature, typename Result, typename Caller, typename ValType>
		void emitTest(std::function<SignalSignature> f, ValType val) {
			Signal<SignalSignature, Result, Caller> sig;
			std::vector<size_t> id(TEST_MAX_SIZE);
			bool timeout = false;
			for (int i = 0; i < TEST_MAX_SIZE + 1; ++i) {
				if (Util::isPow2(i)) {
					{
						std::stringstream ss;
						ss << "Emit time for " << FORMAT(i, TEST_MAX_SIZE) << " connected callbacks";
						if (i > 0) {
							id[i - 1] = sig.connect(f);
						}
						Util::Profiler p(ss.str());
						sig.emit(val);
						sig.results();
						if (p.elapsedInUs() > TEST_TIMEOUT_IN_US) {
							timeout = true;
						}
					}
					{
						std::stringstream ss;
						ss << FORMAT(i, TEST_MAX_SIZE) << " direct calls                     ";
						Util::Profiler p(ss.str());
						for (int j = 0; j < i; ++j) {
							f(val);
						}
						if (p.elapsedInUs() > 2*TEST_TIMEOUT_IN_US) {
							timeout = true;
						}
					}
					if (timeout) {
						std::cout << "TIMEOUT: ABORT CURRENT TEST" << std::endl;
						return;
					}
				} else {
					id[i - 1] = sig.connect(f);
				}
			}
		}

		template<typename SignalSignature, typename Result, typename Caller, typename ValType>
		void emitTestPool(std::function<SignalSignature> f, ValType val) { //TODO: same function as emitTest?
			Signal<SignalSignature, Result, Caller> sig;
			std::vector<size_t> id(TEST_MAX_SIZE);
			bool timeout = false;
			for (int i = 0; i < TEST_MAX_SIZE + 1; ++i) {
				if (Util::isPow2(i)) {
					{
						std::stringstream ss;
						ss << "Emit time for " << FORMAT(i, TEST_MAX_SIZE) << " connected callbacks";
						if (i > 0) {
							id[i - 1] = sig.connect(f);
						}
						ThreadPool pool;
						Util::Profiler p(ss.str());
						sig.emit(val);
						std::function<void(void)> f = [&sig]() { sig.results(true); };
						for (int j = 0; j < i; ++j) {
							pool.submit<void(void)>(f);
						}
						if (p.elapsedInUs() > TEST_TIMEOUT_IN_US) {
							timeout = true;
						}
						pool.WaitForCompletion();
						if (p.elapsedInUs() > TEST_TIMEOUT_IN_US) {
							timeout = true;
						}
					}
					{
						std::stringstream ss;
						ss << FORMAT(i, TEST_MAX_SIZE) << " direct calls                     ";
						Util::Profiler p(ss.str());
						for (int j = 0; j < i; ++j) {
							f(val);
						}
						if (p.elapsedInUs() > 2 * TEST_TIMEOUT_IN_US) {
							timeout = true;
						}
					}
					if (timeout) {
						std::cout << "TIMEOUT: ABORT CURRENT TEST" << std::endl;
						return;
					}
				}
				else {
					id[i - 1] = sig.connect(f);
				}
			}
		}

		int main(int argc, char **argv) {
			Test("create a signal");
			{
				Util::Profiler p("Create void(void)");
				Signal<void(void)> sig;
			}
			{
				Util::Profiler p("Create int(int)");
				for (int i = 0; i < TEST_MAX_SIZE; ++i) {
					Signal<int(int)> sig;
				}
			}
			{
				Util::Profiler p("Create int(int x 8)");
				for (int i = 0; i < TEST_MAX_SIZE; ++i) {
					Signal<int(int, int, int, int, int, int, int, int)> sig;
				}
			}

			Test("connect and disconnect a high number of callbacks on one signal");
			{
				Signal<int(int)> sig;
				std::vector<size_t> id(TEST_MAX_SIZE + 1);
				for (int i = 0; i < TEST_MAX_SIZE + 1; ++i) {
					std::stringstream ss;
					if (Util::isPow2(i)) {
						ss << "Connect number    " << FORMAT(i, TEST_MAX_SIZE);
						Util::Profiler p(ss.str());
						id[i] = sig.connect(Util::dummy);
					}
					else {
						id[i] = sig.connect(Util::dummy);
					}
				}
				for (int i = 0; i < TEST_MAX_SIZE + 1; ++i) {
					std::stringstream ss;
					if (Util::isPow2(i)) {
						ss << "Disconnect number " << FORMAT(i, TEST_MAX_SIZE);
						Util::Profiler p(ss.str());
						bool res = sig.disconnect(id[i]);
						ASSERT(res);
					}
					else {
						bool res = sig.disconnect(id[i]);
						ASSERT(res);
					}
				}
			}

			{
				//dummy
				Test("emit dummy  on async");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerAsync<int(int)>, int>(Util::dummy, 1789);
				Test("emit dummy  on auto");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerAuto<int(int)>, int>(Util::dummy, 1789);
				Test("emit dummy  on pool");
				emitTestPool<int(int), ResultThreadSafeQueue<int>, CallerAuto<int(int)>, int>(Util::dummy, 1789);
				Test("emit dummy  on  sync");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerSync <int(int)>, int>(Util::dummy, 1789);
				Test("emit dummy  on  lazy");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerLazy <int(int)>, int>(Util::dummy, 1789);

				//light computation (~1us)
				Test("emit light computation on async");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerAsync<int(int)>, int>(Util::compute, 12);
				Test("emit light computation on auto");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerAuto<int(int)>, int>(Util::compute, 12);
				Test("emit light computation on pool");
				emitTestPool<int(int), ResultThreadSafeQueue<int>, CallerAuto<int(int)>, int>(Util::compute, 12);
				Test("emit light computation on  sync");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerSync <int(int)>, int>(Util::compute, 12);
				Test("emit light computation on  lazy");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerLazy <int(int)>, int>(Util::compute, 12);

				//light computation (~300us)
				Test("emit heavy computation on async");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerAsync<int(int)>, int>(Util::compute, 27);
				Test("emit heavy computation on auto");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerAuto<int(int)>, int>(Util::compute, 27);
				Test("emit heavy computation on pool");
				emitTestPool<int(int), ResultThreadSafeQueue<int>, CallerAuto<int(int)>, int>(Util::compute, 27);
				Test("emit heavy computation on  sync");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerSync <int(int)>, int>(Util::compute, 27);
				Test("emit heavy computation on  lazy");
				emitTest<int(int), ResultThreadSafeQueue<int>, CallerLazy <int(int)>, int>(Util::compute, 27);

				//sleep
				Test("emit sleep   on async");
				emitTest<void(int), ResultThreadSafeQueue<void>, CallerAsync<void(int)>, int>(Util::sleepInMs, 100);
				Test("emit sleep   on  auto");
				emitTest<void(int), ResultThreadSafeQueue<void>, CallerAuto<void(int)>, int>(Util::sleepInMs, 100);
				Test("emit sleep   on  pool");
				emitTestPool<void(int), ResultThreadSafeQueue<void>, CallerAuto<void(int)>, int>(Util::sleepInMs, 100);
				Test("emit sleep   on  sync");
				emitTest<void(int), ResultThreadSafeQueue<void>, CallerSync <void(int)>, int>(Util::sleepInMs, 100);
				Test("emit sleep   on  lazy");
				emitTest<void(int), ResultThreadSafeQueue<void>, CallerLazy <void(int)>, int>(Util::sleepInMs, 100);
			}

			return 0;
		}
	}
}

#ifdef UNIT
using namespace Tests;
int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");

	int res = 0;

	res = Perf::main(argc, argv);
	ASSERT(!res);

	std::cout << std::endl;
	return 0;
}
#endif
