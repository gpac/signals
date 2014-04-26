#pragma once

#ifdef _WIN32
#ifdef MM_COMPILATION
#define MM_EXPORT __declspec(dllexport)
#else
#define MM_EXPORT __declspec(dllimport)
#endif
#else
#define MM_EXPORT
#endif
