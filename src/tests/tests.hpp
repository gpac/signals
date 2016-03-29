#pragma once

#ifdef _SIGNALS_HPP_
#error Please include tests.hpp before signals.hpp
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include <csignal>
#include <cstdint>
#include <iostream>
#include <sstream>


#define TESTS

#ifdef _WIN32
#define SLEEP_IN_MS(ms) Sleep(ms)
#else
//#define SLEEP_IN_MS(ms) usleep(ms)
#define SLEEP_IN_MS(ms) { struct timespec t; t.tv_sec=ms/1000; t.tv_nsec=(ms-t.tv_sec*1000)*1000000; nanosleep(&t, nullptr); };
#endif


#define TEST_MAX_SIZE (1<<12)
#define TEST_TIMEOUT_IN_US 800000

// generate a file-unique identifier, based on current line
#define unittestSuffix(suffix, prettyName) \
	static void testFunction##suffix(); \
	int g_isRegistered##suffix = Tests::RegisterTest(&testFunction##suffix, prettyName, g_isRegistered##suffix); \
	static void testFunction##suffix()

#define unittestLine(counter, prettyName) \
	unittestSuffix(counter, prettyName)

#define unittest(prettyName) \
	unittestLine(__COUNTER__, prettyName)

namespace Tests {
inline void Fail(char const* file, int line, const char* msg) {
	std::cerr << "TEST FAILED: " << file << "(" << line << "): " << msg << std::endl;
	std::raise(SIGABRT);
}

#define ASSERT(expr) \
	if(!(expr)) { \
		std::stringstream exprStringStream; \
		exprStringStream << "assertion failed: " << #expr; \
		Fail(__FILE__, __LINE__, exprStringStream.str().c_str()); \
 	}

#define ASSERT_EQUALS(expected, actual) \
	if((expected) != (actual)) { \
		std::stringstream ss; \
		ss << "assertion failed for expression: '" << #actual << "' , expected '" << (expected) << "' got '" << (actual) << "'"; \
		Fail(__FILE__, __LINE__, ss.str().c_str()); \
	}

int RegisterTest(void (*f)(), const char* testName, int& dummy);
void RunAll();

inline void Test(const std::string &name) {
	std::cout << std::endl << "[ ***** " << name.c_str() << " ***** ]" << std::endl;
}

namespace Util {
inline int dummy(int a) {
	return a;
}
inline int dummyPrint(int a) {
	std::cout << "a = " << a << std::endl;
	return a;
}

inline int compute(int a) {
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

inline void sleepInMs(int ms) {
	//std::cout << "sleepInMs(" << ms << ") in thread " << std::this_thread::get_id() << std::endl;
	SLEEP_IN_MS(ms);
}

inline int log2(int i) {
	int res = 0;
	while (i >>= 1) {
		++res;
	}
	return res;
}

inline bool isPow2(int i) {
	return (i == 0) || (i - (1 << (int)log2(i)) == 0);
}

}
}
