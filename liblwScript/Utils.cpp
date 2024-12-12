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
        file.close();
        return sstream.str();
    }

    void WriteBinaryFile(std::string_view path, const std::vector<uint8_t> &content)
    {
        std::ofstream file;
        file.open(path.data(), std::ios::out | std::ios::binary);
        if (!file.is_open())
#ifdef USE_UTF8_ENCODE
        {
            auto utf8Path = Utf8Decode(path.data());
            Logger::Error(TEXT("Failed to open file:{}"), utf8Path);
        }
#else
            Logger::Error(TEXT("Failed to open file:{}"), path);
#endif
        file.write((const char *)content.data(), content.size());
        file.close();
    }

    std::vector<uint8_t> LWSCRIPT_API ReadBinaryFile(std::string_view path)
    {
        std::ifstream file;
        file.open(path.data(), std::ios::in | std::ios::binary);
        if (!file.is_open())
#ifdef USE_UTF8_ENCODE
        {
            auto utf8Path = Utf8Decode(path.data());
            Logger::Error(TEXT("Failed to open file:{}"), utf8Path);
        }
#else
            Logger::Error(TEXT("Failed to open file:{}"), path);
#endif

        std::stringstream sstream;
        sstream << file.rdbuf();
        file.close();
        auto str = sstream.str();

        std::vector<uint8_t> result;
        result.assign(str.begin(), str.end());

        return result;
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

    uint64_t NormalizeIdx(int64_t idx, size_t dsSize)
    {
        bool isNeg = false;
        if (idx < 0)
        {
            isNeg = true;
            idx = abs(idx);
        }

        idx = idx % (int64_t)dsSize;

        if (isNeg)
            idx = dsSize - idx;

        return idx;
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