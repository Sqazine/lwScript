#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>

namespace lws
{
    inline void Assert(std::string_view msg)
    {
        std::cout << msg << std::endl;
#ifndef _DEBUG
        assert(0);
#else
        exit(1);
#endif
    }

    inline std::string ReadFile(std::string_view path)
    {
        std::fstream file;
        file.open(path.data(), std::ios::in | std::ios::binary);
        if (!file.is_open())
            Assert("Failed to open file:" + std::string(path));

        std::stringstream sstream;
        sstream << file.rdbuf();
        return sstream.str();
    }

    inline std::string PointerAddressToString(void *pointer)
    {
        std::stringstream sstr;
        sstr << pointer;
        std::string address = sstr.str();
        return address;
    }
}