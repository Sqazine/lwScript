#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <array>

#define STACK_MAX 512
#define GLOBAL_VARIABLE_MAX 512

#define UINT8_COUNT (UINT8_MAX + 1)

#define GC_HEAP_GROW_FACTOR 2

#ifndef LWSCRIPT_BUILD_STATIC
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
#else
#define LWSCRIPT_API
#endif

#ifdef LWSCRIPT_UTF8_ENCODE
#define STR(x) L##x
#define TEXT(x) STR(x)
#define TCHAR(x) STR(x)
#define CHAR_T wchar_t
#define STRING std::wstring
#define STRING_VIEW std::wstring_view
#define OSTREAM std::wostream
#define STRING_STREAM std::wstringstream
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
#define STRING std::string
#define STRING_VIEW std::string_view
#define OSTREAM std::ostream
#define STRING_STREAM std::stringstream
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
#define SINGLETON_DECL(T)             \
	static T *GetInstance() noexcept; \
	NON_COPYABLE(T)

#define SINGLETON_IMPL(T)        \
	T *T::GetInstance() noexcept \
	{                            \
		static T sInstance;      \
		return &sInstance;       \
	}

#define SAFE_DELETE(x)   \
	do                   \
	{                    \
		if (x)           \
		{                \
			delete x;    \
			x = nullptr; \
		}                \
	} while (false);

namespace lwscript
{
	enum Permission : uint8_t
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

	STRING LWSCRIPT_API ReadFile(std::string_view path);
	void LWSCRIPT_API WriteBinaryFile(std::string_view path, const std::vector<uint8_t> &content);
	std::vector<uint8_t> LWSCRIPT_API ReadBinaryFile(std::string_view path);

	STRING PointerAddressToString(void *pointer);

	int64_t Factorial(int64_t v, int64_t tmp = 1);

	uint64_t NormalizeIdx(int64_t idx, size_t dsSize);

	namespace Utf8
	{
		std::string Encode(const std::wstring &str);
		std::wstring Decode(const std::string &str);
	}

	namespace ByteConverter
	{
		std::array<uint8_t, 8> ToU64ByteList(int64_t integer);
		uint64_t GetU64Integer(const std::vector<uint8_t> data, uint32_t start);

		std::array<uint8_t, 4> ToU32ByteList(int32_t integer);
		uint32_t GetU32Integer(const std::vector<uint8_t> data, uint32_t start);
	}
}