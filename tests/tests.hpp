#ifdef _MSC_VER
#include <Windows.h>
#else
#error not implemented
#endif

#include <iostream>

namespace Tests {
	//class Test {
	//public:
		void Test(const std::string &name) {
			std::cout << std::endl << "[ ***** " << name.c_str() << " ***** ]" << std::endl;
		}

		//TODO test result in destuctor?
	//};

namespace Util {
	bool isPow2(int i) {
		return (i == 0) || (i - ((1 << (int)std::log2(i))) == 0);
	}

	class Profiler {
	public:
		Profiler(const std::string &name) : name(name) {
			QueryPerformanceCounter(&startTime);
		}

		~Profiler() {
			std::cout << "[" << name.c_str() << "] " << elapsedInUs() << " us" << std::endl;
		}

		uint64_t elapsedInUs() {
			LARGE_INTEGER stopTime;
			QueryPerformanceCounter(&stopTime);
			LARGE_INTEGER countsPerSecond;
			QueryPerformanceFrequency(&countsPerSecond);
			return (uint64_t)((1000000 * (stopTime.QuadPart - startTime.QuadPart)) / countsPerSecond.QuadPart);
		}

	private:
		std::string name;
		LARGE_INTEGER startTime;
	};
}
}