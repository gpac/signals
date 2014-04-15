#include "clock.hpp"
#include <chrono>

namespace Modules {

	using namespace std::chrono;

	class SystemClock : public IClock {
	public:
		SystemClock() : m_Start(high_resolution_clock::now()){
		}
		virtual uint64_t now() const {
			auto const timeNow = high_resolution_clock::now();
			auto const timeNowInMs = duration_cast<milliseconds>(timeNow - m_Start);
			return timeNowInMs.count();
		}
	private:
		time_point<high_resolution_clock> const m_Start;
	};

	IClock* createSystemClock() {
		return new SystemClock;
	}

	static SystemClock systemClock;
	extern IClock* const g_DefaultClock = &systemClock;
}
