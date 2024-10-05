#pragma once

#define CONSTANT_MAX 512
#define STACK_MAX 512
#define GLOBAL_VARIABLE_MAX 512

#define UINT8_COUNT (UINT8_MAX + 1)

#define GC_HEAP_GROW_FACTOR 2

#define USE_FUNCTION_CACHE

#ifdef _DEBUG
//#define GC_DEBUG
//#define GC_STRESS
#define PRINT_FUNCTION_CACHE
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifdef LWSCRIPT_BUILD_DLL
#define LWSCRIPT_API __declspec(dllexport)
#else
#define LWSCRIPT_API __declspec(dllimport)
#endif
#else
#ifdef LWSCRIPT_BUILD_DLL
#define LWSCRIPT_API
#else
#define LWSCRIPT_API
#endif

#endif