#include <sstream>
#include <vector>


#define MAX_TEST_SIZE (2<<13)


namespace Tests {
	namespace Perf {
		int dummy(int a) {
			return a;
		}

		int main(int argc, char **argv) {
			//create a signal
			{
				Util::Profiler p("Create void(void)");
				Signal<void(void)> sig;
			}
			{
				Util::Profiler p("Create int(int)");
				for (int i = 0; i < MAX_TEST_SIZE; ++i) {
					Signal<int(int)> sig;
				}
			}
			{
			Util::Profiler p("Create int(int x 8)");
			for (int i = 0; i < MAX_TEST_SIZE; ++i) {
				Signal<int(int, int, int, int, int, int, int, int)> sig;
			}
		}

			//connect, and disconnect a high number of callbacks on one signal
			{
				Signal<int(int)> sig;
				std::vector<size_t> id(MAX_TEST_SIZE + 1);
				for (int i = 0; i < MAX_TEST_SIZE + 1; ++i) {
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
				for (int i = 0; i < MAX_TEST_SIZE + 1; ++i) {
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

			//emit with a high number of connected callbacks on one signal
			{
				Signal<int(int)> sig;
				std::vector<size_t> id(MAX_TEST_SIZE + 1);
				for (int i = 0; i < MAX_TEST_SIZE + 1; ++i) {
					if (Util::isPow2(i)) {
						{
							std::stringstream ss;
							ss << "Emit time for " << i << " connected callbacks";
							id[i] = sig.connect(dummy);
							Util::Profiler p(ss.str());
							sig.emit(100);
						}
						{
							std::stringstream ss; 
							ss << i << " direct calls                     ";
							Util::Profiler p(ss.str());
							for (int j = 0; j < i; ++j) {
								dummy(100);
							}
						}
					}
					else {
						id[i] = sig.connect(dummy);
					}
				}
			}

			//emit synchronous with a high number of connected callbacks on one signal
			{
				Signal<int(int), ResultVector<int>, CallerSync<ResultVector<int>, int(int)>> sig;
				std::vector<size_t> id(MAX_TEST_SIZE + 1);
				for (int i = 0; i < MAX_TEST_SIZE + 1; ++i) {
					if (Util::isPow2(i)) {
						{
							std::stringstream ss;
							ss << "Emit time for " << i << " connected callbacks";
							id[i] = sig.connect(dummy);
							Util::Profiler p(ss.str());
							sig.emit(100);
						}
						{
						std::stringstream ss;
						ss << i << " direct calls                     ";
						Util::Profiler p(ss.str());
						for (int j = 0; j < i; ++j) {
							dummy(100);
						}
					}
					}
					else {
						id[i] = sig.connect(dummy);
					}
				}
			}

			return 0;
		}
	}
}
