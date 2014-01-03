#ifdef _MSC_VER
#include <Windows.h>
#else
#error not implemented
#endif

#include <iostream>

namespace Tests {
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
			LARGE_INTEGER stopTime;
			QueryPerformanceCounter(&stopTime);
			LARGE_INTEGER countsPerSecond;
			QueryPerformanceFrequency(&countsPerSecond);
			uint64_t duration = (uint64_t)((1000000 * (stopTime.QuadPart - startTime.QuadPart)) / countsPerSecond.QuadPart);
			std::cout << "[" << name.c_str() << "] " << duration << " us" << std::endl;
		}

	private:
		std::string name;
		LARGE_INTEGER startTime;
	};
}
}