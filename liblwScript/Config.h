#pragma once

#define CONSTANT_MAX 512
#define STACK_MAX 512
#define GLOBAL_VARIABLE_MAX 512

#define UINT8_COUNT (UINT8_MAX + 1)

#define GC_HEAP_GROW_FACTOR 2

#define MAIN_ENTRY_FUNCTION_NAME L"_main_start_up"

#ifndef NDEBUG
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

namespace lwscript
{
    class LWSCRIPT_API Config
    {
        NON_COPYABLE(Config)
    public:
        SINGLETON_DECL(Config)

        void SetIsUseFunctionCache(bool v);
        bool IsUseFunctionCache() const;

    private:
        Config() = default;
        ~Config() = default;

        bool mUseFunctionCache{false};
    };
}