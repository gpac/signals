#pragma once

#include <cstring>
#include <memory>
#include <vector>

// Runs a function at instanciation:
// Use to initialize C libraries at program startup.
// Example:
// auto g_InitAv = runAtStartup(&av_register_all);
struct DummyStruct {
};

template<class R, class... Args>
DummyStruct runAtStartup(R f(Args...), Args... argVal) {
	f(argVal...);
	return DummyStruct();
}

inline
std::vector<char> stringDup(const char *src) {
	std::vector<char> data(strlen(src) + 1);
	strcpy(data.data(), src);
	return data;
}

template<typename T>
T divUp(T num, T divisor) {
	return (num + divisor - 1) / divisor;
}

template<typename T>
std::vector<T> makeVector() {
	return std::vector<T>();
}

template<typename T>
std::vector<T> makeVector(T val) {
	std::vector<T> r;
	r.push_back(val);
	return r;
}

template<typename T, typename... Arguments>
std::vector<T> makeVector(T val, Arguments... args) {
	std::vector<T> r;
	r.push_back(val);
	auto const tail = makeVector(args...);
	r.insert(r.end(), tail.begin(), tail.end());
	return r;
}

#define foreach(iterator, container) \
	for(auto iterator=std::begin(container);iterator != std::end(container);++iterator)

template<typename Lambda, typename T>
std::vector<T> apply(Lambda func, std::vector<T>& input) {
	std::vector<T> r;
	foreach(element, input)
	r.push_back(func(*element));
	return r;
}

template<typename T>
std::unique_ptr<T> uptr(T* p) {
	return std::unique_ptr<T>(p);
}
#define UPTR_DEFINED
