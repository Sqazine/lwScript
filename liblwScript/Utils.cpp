#include "Utils.h"
#include "Logger.h"
#include <locale>
#include <codecvt>
#include <fstream>
#include <sstream>
namespace lwscript
{
    STD_STRING ReadFile(std::string_view path)
    {
#ifdef USE_UTF8_ENCODE
        auto utf8Path = Utf8Decode(path.data());
        Logger::Record::mCurFilePath = utf8Path;
#else
        Logger::Record::mCurFilePath = path;
#endif

        STD_IFSTREAM file;
        file.open(Logger::Record::mCurFilePath.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
            Logger::Error(TEXT("Failed to open file:{}"), Logger::Record::mCurFilePath);

        STD_STRING_STREAM sstream;
        sstream << file.rdbuf();
        return sstream.str();
    }

    STD_STRING PointerAddressToString(void *pointer)
    {
        STD_STRING_STREAM sstr;
        sstr << pointer;
        STD_STRING address = sstr.str();
        return address;
    }

    int64_t Factorial(int64_t v, int64_t tmp)
    {
        if (v == 0)
            return tmp;
        return Factorial(v - 1, v * tmp);
    }

    std::string Utf8Encode(const std::wstring &str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(str);
    }

    std::wstring Utf8Decode(const std::string &str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    }
}