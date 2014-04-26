#pragma once

#ifdef _WIN32
#ifdef MODULES_COMPILATION
#define MODULES_EXPORT __declspec(dllexport)
#else
#define MODULES_EXPORT __declspec(dllimport)
#endif
#else
#define MODULES_EXPORT
#endif
