#include <sstream>
#include <vector>


#define TEST_SIZE (2<<11)
#define TEST_TIMEOUT_IN_US 7000000


namespace Tests {
	namespace Perf {
		template<typename SignalSignature, typename Result, typename Caller>
		void emitTest(std::function<SignalSignature> f) {
			Signal<SignalSignature, Result, Caller> sig;
			std::vector<size_t> id(TEST_SIZE + 1);
			bool timeout = false;
			for (int i = 0; i < TEST_SIZE + 1; ++i) {
				if (Util::isPow2(i)) {
					const int val = 28;
					{
						std::stringstream ss;
						ss << "Emit time for " << i << " connected callbacks";
						id[i] = sig.connect(f);
						Util::Profiler p(ss.str());
						sig.emit(val);
						sig.results();
						if (p.elapsedInUs() > TEST_TIMEOUT_IN_US) {
							timeout = true;
						}
					}
					{
						std::stringstream ss;
						ss << i << " direct calls                     ";
						Util::Profiler p(ss.str());
						for (int j = 0; j < i; ++j) {
							f(val);
						}
					}
					if (timeout) {
						std::cout << "TIMEOUT: ABORT CURRENT TEST" << std::endl;
						return;
					}
				} else {
					id[i] = sig.connect(f);
				}
			}
		}

		int dummy(int a) {
			return a;
		}

		int compute(int a) {
			int64_t n = (int64_t)1 << a;
			if (a <= 0) {
				return 1;
			}
			uint64_t res = n;
			while (--n > 1) {
				res *= n;
			}
			return (int)res;
		}

		int main(int argc, char **argv) {
			Test("create a signal");
			{
				Util::Profiler p("Create void(void)");
				Signal<void(void)> sig;
			}
			{
				Util::Profiler p("Create int(int)");
				for (int i = 0; i < TEST_SIZE; ++i) {
					Signal<int(int)> sig;
				}
			}
			{
				Util::Profiler p("Create int(int x 8)");
				for (int i = 0; i < TEST_SIZE; ++i) {
					Signal<int(int, int, int, int, int, int, int, int)> sig;
				}
			}

			Test("connect, and disconnect a high number of callbacks on one signal");
			{
				Signal<int(int)> sig;
				std::vector<size_t> id(TEST_SIZE + 1);
				for (int i = 0; i < TEST_SIZE + 1; ++i) {
					std::stringstream ss;
					if (Util::isPow2(i)) {
						ss << "Connect number " << i;
						Util::Profiler p(ss.str());
						id[i] = sig.connect(dummy);
					}
					else {
						id[i] = sig.connect(dummy);
					}
				}
				for (int i = 0; i < TEST_SIZE + 1; ++i) {
					std::stringstream ss;
					if (Util::isPow2(i)) {
						ss << "Disconnect number " << i;
						Util::Profiler p(ss.str());
						bool res = sig.disconnect(id[i]);
						assert(res);
					}
					else {
						bool res = sig.disconnect(id[i]);
						assert(res);
					}
				}
			}

			Test("emit dummy  on async");
			emitTest<int(int), ResultVector<int>, CallerAsync<ResultVector<int>, int(int)>>(dummy);
			Test("emit dummy  on  sync");
			emitTest<int(int), ResultVector<int>, CallerSync <ResultVector<int>, int(int)>>(dummy);
			Test("emit compute on async");
			emitTest<int(int), ResultVector<int>, CallerAsync<ResultVector<int>, int(int)>>(compute);
			Test("emit compute on  sync");
			emitTest<int(int), ResultVector<int>, CallerSync <ResultVector<int>, int(int)>>(compute);

			return 0;
		}
	}
}
