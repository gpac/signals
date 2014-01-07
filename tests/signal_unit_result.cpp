#include "signals.hpp"
#include "tests.hpp"

#include "internal/core/result.hpp"


namespace Tests {
	namespace Unit {
		namespace Result {
			template<typename T>
			bool test() {
				ResultThreadSafeQueue<T> result;
				auto res = result.get();

				return true;
			}

			int main(int argc, char **argv) {
				//TODO: complete this test... and create other unit tests
				Test("unit test on class Result");
				{
					bool res = test<int>();
					ASSERT(res);
				}
				{
					bool res = test<void>();
					ASSERT(res);
				}

				return 0;
			}
		}
	}
}
