#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <array>
#include "Config.h"
#include "Logger.h"
namespace lwscript
{
#define SAFE_DELETE(x)   \
    do                   \
    {                    \
        if (x)           \
        {                \
            delete x;    \
            x = nullptr; \
        }                \
    } while (false);

	enum Privilege
	{
		MUTABLE,
		IMMUTABLE,
	};

	enum VarArg
	{
		NONE = 0,
		WITHOUT_NAME,
		WITH_NAME,
	};

	STD_STRING LWSCRIPT_API ReadFile(std::string_view path);

	STD_STRING PointerAddressToString(void *pointer);

	int64_t Factorial(int64_t v, int64_t tmp = 1);

	std::string Utf8Encode(const std::wstring &str);
	std::wstring Utf8Decode(const std::string &str);
}