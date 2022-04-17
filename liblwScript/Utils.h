#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <cassert>
namespace lws
{
    inline void Assert(std::wstring_view msg)
    {
        std::wcout << msg << std::endl;
#ifdef _DEBUG
        assert(0);
#else
        exit(1);
#endif
    }

	inline void Assert(std::string_view msg)
	{
		std::cout << msg << std::endl;
#ifdef _DEBUG
		assert(0);
#else
		exit(1);
#endif
	}

    inline std::wstring ReadFile(std::string_view path)
    {
        std::wifstream file;
        file.open(path.data(), std::ios::in | std::ios::binary);
        if (!file.is_open())
            Assert("Failed to open file:"+std::string(path));

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

    inline int64_t Factorial(int64_t v)
    {
        if(v==0)
            return 1;
        return Factorial(v - 1) * v;
    }
}