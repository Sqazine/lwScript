#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <cassert>
#include <vector>
#include <array>
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

    inline int64_t Factorial(int64_t v, int64_t tmp = 1)
    {
        if (v == 0)
            return tmp;
        return Factorial(v - 1, v * tmp);
    }

    inline std::array<uint8_t, 8> DecodeUint64(uint64_t ui)
    {
        std::array<uint8_t, 8> result;
        result[0] = uint8_t(ui >> 56);
        result[1] = uint8_t(ui >> 48);
        result[2] = uint8_t(ui >> 40);
        result[3] = uint8_t(ui >> 32);
        result[4] = uint8_t(ui >> 24);
        result[5] = uint8_t(ui >> 16);
        result[6] = uint8_t(ui >> 8);
        result[7] = uint8_t(ui >> 0);

        return result;
    }

    inline uint64_t EncodeUint64(const std::vector<uint8_t>& byteList,int64_t idx)
    {
        return (((uint64_t)byteList[idx+1]) << 56) |
               (((uint64_t)byteList[idx+2]) << 48) |
               (((uint64_t)byteList[idx+3]) << 40) |
               (((uint64_t)byteList[idx+4]) << 32) |
               (((uint64_t)byteList[idx+5]) << 24) |
               (((uint64_t)byteList[idx+6]) << 16) |
               (((uint64_t)byteList[idx+7]) << 8) |
               (((uint64_t)byteList[idx+8]) << 0);
    }
}