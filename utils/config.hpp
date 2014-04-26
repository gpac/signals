#pragma once

#ifdef _WIN32
#ifdef UTILS_COMPILATION
#define UTILS_EXPORT __declspec(dllexport)
#else
#define UTILS_EXPORT __declspec(dllimport)
#endif
#else
#define UTILS_EXPORT
#endif
