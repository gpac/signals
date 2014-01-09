#pragma once

#ifdef MODULES_COMPILATION
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
