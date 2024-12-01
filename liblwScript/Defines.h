#pragma once

#define STACK_MAX 512
#define GLOBAL_VARIABLE_MAX 512

#define UINT8_COUNT (UINT8_MAX + 1)

#define GC_HEAP_GROW_FACTOR 2

#ifndef NDEBUG
#define GC_DEBUG
#define GC_STRESS
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

#ifdef USE_UTF8_ENCODE
#define STR(x) L##x
#define TEXT(x) STR(x)
#define TCHAR(x) STR(x)
#define CHAR_T wchar_t
#define STD_STRING std::wstring
#define STD_STRING_VIEW std::wstring_view
#define STD_OSTREAM std::wostream
#define STD_STRING_STREAM std::wstringstream
#define STD_IFSTREAM std::wifstream
#define TO_STRING(x) std::to_wstring(x)
#define COUT std::wcout
#define CIN std::wcin
#define STRCMP wcscmp
#else
#define STR(x) x
#define TEXT(x) STR(x)
#define TCHAR(x) STR(x)
#define CHAR_T char
#define STD_STRING std::string
#define STD_STRING_VIEW std::string_view
#define STD_OSTREAM std::ostream
#define STD_STRING_STREAM std::stringstream
#define STD_IFSTREAM std::ifstream
#define TO_STRING(x) std::to_string(x)
#define COUT std::cout
#define CIN std::cin
#define STRCMP strcmp
#endif

#define MAIN_ENTRY_FUNCTION_NAME TEXT("_main_start_up")

#define NON_COPYABLE(T)                     \
    T(const T &) = delete;                  \
    T(T &&) = delete;                       \
    const T &operator=(const T &) = delete; \
    const T &operator=(T &&) = delete;

//  Meyers' Singleton
#define SINGLETON_DECL(T) static T *GetInstance() noexcept;
#define SINGLETON_IMPL(T)        \
    T *T::GetInstance() noexcept \
    {                            \
        static T sInstance;      \
        return &sInstance;       \
    }
