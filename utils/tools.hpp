#pragma once

// Runs a function at instanciation:
// Use to initialize C libraries at program startup.
// Example:
// auto g_InitAv = runAtStartup(&av_register_all);
struct DummyStruct {
};

template<class R, class... Args>
DummyStruct runAtStartup(R f(Args...), Args... argVal)
{
	f(argVal...);
	return DummyStruct();
}

template<typename T>
std::unique_ptr<T> uptr(T* p)
{
	return std::unique_ptr<T>(p);
}
