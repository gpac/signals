#include "tests.hpp"

#include "../utils/tools.hpp"
#include "../utils/log.hpp"

using namespace Tests;

namespace {

unittest("makeVector: empty") {
	auto const testVector = makeVector<int>();
	ASSERT(testVector.empty());
}

unittest("makeVector: 4 elements") {
	auto const testVector = makeVector(1, 3, 5, 7);
	ASSERT(testVector[0] == 1);
	ASSERT(testVector[1] == 3);
	ASSERT(testVector[2] == 5);
	ASSERT(testVector[3] == 7);
	ASSERT(testVector.size() == 4);
}

unittest("makeVector: vector elements") {
	auto const smallVector = makeVector(1, 2);
	auto const bigVector = makeVector(smallVector, smallVector);

	ASSERT(bigVector.size() == 2);
	for(size_t i=0; i < bigVector.size(); ++i) {
		ASSERT(bigVector[i][0] == 1);
		ASSERT(bigVector[i][1] == 2);
	}
}

unittest("format: one argument") {
	ASSERT_EQUALS("45", format("%s", 45));
}

unittest("format: one uint8_t argument") {
	ASSERT_EQUALS("45", format("%s", (uint8_t)45));
}

unittest("format: one char argument") {
	ASSERT_EQUALS("A", format("%s", 'A'));
}

unittest("format: vector argument") {
	std::vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	auto const s = format("%s", v);
	ASSERT(s == "[1, 2, 3]");
}

unittest("format: vector argument") {
	std::vector<std::vector<int>> v;
	v.resize(3);
	v[1].resize(2);
	v[1][1] = 7;
	v[2].resize(1);
	auto const s = format("%s", v);
	ASSERT(s == "[[], [0, 7], [0]]");
}

unittest("stringDup") {
	const char c[] = "01234";
	auto s = stringDup(c);
	ASSERT(!memcmp(s.data(), c, s.size()));
}

}
