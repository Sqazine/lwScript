#include "Utils.h"
#include <locale>
#include <codecvt>
namespace lwscript
{
    std::wstring ReadFile(std::string_view path)
    {
        Hint::Record::mCurFilePath = Utf8Decode(path.data());

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

    std::wstring PointerAddressToString(void *pointer)
    {
        std::wstringstream sstr;
        sstr << pointer;
        std::wstring address = sstr.str();
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