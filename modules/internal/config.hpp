#pragma once

#ifdef WIN32
#ifdef MODULES_COMPILATION
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
#else
#define EXPORT
#endif
