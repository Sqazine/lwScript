#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <cassert>
namespace lws
{
#ifdef _DEBUG
#define ASSERT(msg)                         \
    {                                       \
        do                                  \
        {                                   \
            std::wcout << msg << std::endl; \
            assert(0);                      \
        } while (false);                    \
    }
#else
#define ASSERT(msg)                         \
    {                                       \
        do                                  \
        {                                   \
            std::wcout << msg << std::endl; \
            exit(1);                        \
        } while (false);                    \
    }
#endif

    inline std::wstring ReadFile(std::string_view path)
    {
        std::wifstream file;
        file.open(path.data(), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            std::cout << "Failed to open file:" << path << std::endl;
#ifdef _DEBUG
            assert(0);
#else
            exit(1);
#endif
        }

        std::wstringstream sstream;
        sstream << file.rdbuf();
        return sstream.str();
    }

    inline std::wstring PointerAddressToString(void *pointer)
    {
        std::wstringstream sstr;
        sstr << pointer;
        std::wstring address = sstr.str();
        return address;
    }

    inline int64_t Factorial(int64_t v, int64_t tmp=1)
    {
        if (v == 0)
            return tmp;
        return Factorial(v - 1, v * tmp);
    }
}