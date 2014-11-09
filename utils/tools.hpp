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
	const size_t srcStrLen = strlen(src) + 1;
	std::vector<char> data(srcStrLen);
	strncpy(data.data(), src, srcStrLen);
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

template<typename Lambda, typename T>
std::vector<T> apply(Lambda func, std::vector<T>& input) {
	std::vector<T> r;
	for(auto& element : input)
		r.push_back(func(element));
	return r;
}

template<typename T>
std::unique_ptr<T> uptr(T* p) {
	return std::unique_ptr<T>(p);
}

template<typename T, typename U>
std::shared_ptr<T> safe_cast(std::shared_ptr<U> p)
{
	auto r = std::dynamic_pointer_cast<T>(p);
	if(!r)
	{
		std::string s;
		s += "dynamic cast error: could not convert from ";
		s += typeid(U).name();
		s += " to ";
		s += typeid(T).name();
		throw std::runtime_error(s.c_str());
	}
	return r;
}
