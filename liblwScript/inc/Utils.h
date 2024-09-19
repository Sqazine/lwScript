#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <string>
#include <cassert>
#include <cstdarg>
#include <vector>
#include <array>
#include "Token.h"
namespace lwscript
{
    enum class Privilege
    {
        MUTABLE,
        IMMUTABLE,
    };

    std::wstring LWSCRIPT_API ReadFile(std::string_view path);

    std::wstring PointerAddressToString(void *pointer);

    int64_t Factorial(int64_t v, int64_t tmp = 1);

    std::string Utf8Encode(const std::wstring &str);
    std::wstring Utf8Decode(const std::string &str);

    inline void Log(std::wostream &os, std::wstring_view s)
    {
        os << s;
    }

    template <typename... Args>
    inline void Log(std::wostream &os, std::wstring_view s, const Args &...args)
    {
        Log(os, s, args...);
    }

    template <typename T, typename... Args>
    inline void Log(std::wostream &os, std::wstring_view s, const T &next, const Args &...args)
    {
        auto index = s.find_first_of(L"{}");
        if (index == std::wstring::npos)
            Log(os, s);
        else
        {
            std::wstring tmpS = s.data();
            std::wstringstream sstr;
            sstr << next;
            tmpS.replace(index, 2, sstr.str());
            Log(os, tmpS, args...);
        }
    }

    template <typename... Args>
    inline void Println(std::wstring_view s, const Args &...args)
    {
        auto withLn = std::wstring(s) + L"\n";
        Log(std::wcout, withLn, args...);
    }

    template <typename... Args>
    inline void Print(std::wstring_view s, const Args &...args)
    {
        Log(std::wcout, s, args...);
    }

    namespace Hint
    {
        namespace Record
        {
            inline std::wstring mCurFilePath = L"interpreter";
            inline std::wstring mSourceCode = L"";
        }

        inline void RecordSource(std::wstring_view sourceCode)
        {
            Record::mSourceCode = sourceCode;
        }

        template <typename... Args>
        inline void AssemblyLogInfo(const std::wstring &headerHint, const std::wstring &colorHint, uint64_t lineNum, uint64_t column, uint64_t pos, const std::wstring &fmt, const Args &...args)
        {
            auto start = pos;
            auto end = pos;

            if (Record::mCurFilePath != L"interpreter")
            {
                while ((Record::mSourceCode[start - 1] != L'\n' && Record::mSourceCode[start - 1] != L'\r') && start - 1 > 0)
                    start--;

                while ((Record::mSourceCode[end] != L'\n' && Record::mSourceCode[end] != L'\r') && end < Record::mSourceCode.size())
                    end++;
            }
            else
            {
                start = 0;
                end = Record::mSourceCode.size();
            }

            auto startStr = headerHint + L":" + Record::mCurFilePath + L"(line " + std::to_wstring(lineNum) + L",column " + std::to_wstring(column) + L"): ";

            auto lineSrcCode = Record::mSourceCode.substr(start, end - start);

            Println(L"\033[{}m{}{}\033[0m", colorHint, startStr, lineSrcCode);

            auto blankSize = startStr.size() + pos - start;

            std::wstring errorHintStr;
            errorHintStr.insert(0, blankSize, L' ');
            errorHintStr += L"^ " + std::wstring(fmt);

            Println(L"\033[{}m" + errorHintStr + L"\033[0m", colorHint, args...);
        }

        template <typename... Args>
        inline void Error(uint64_t pos, const std::wstring &fmt, const Args &...args)
        {
            auto lineNum = 1;
            for (uint64_t i = 0; i < pos; ++i)
                if (Record::mSourceCode[i] == L'\n' || Record::mSourceCode[i] == L'\r')
                    lineNum++;

            AssemblyLogInfo(L"[ERROR]", L"31", lineNum, 1, pos, fmt, args...);
#ifdef _DEBUG
            assert(0);
#else
            exit(1);
#endif
        }
        template <typename... Args>
        inline void Error(const Token *tok, const std::wstring &fmt, const Args &...args)
        {
            AssemblyLogInfo(L"[ERROR]", L"31", tok->line, tok->column, tok->pos, fmt, args...);
#ifdef _DEBUG
            assert(0);
#else
            exit(1);
#endif
        }

        template <typename... Args>
        void Warn(int32_t pos, const std::wstring &fmt, const Args &...args)
        {
            auto lineNum = 1;
            for (int32_t i = 0; i < pos; ++i)
                if (Record::mSourceCode[i] == L'\n' || Record::mSourceCode[i] == L'\r')
                    lineNum++;

            AssemblyLogInfo(L"[WARN]", L"33", lineNum, 1, pos, fmt, args...);
        }

        template <typename... Args>
        void Warn(Token tok, const std::wstring &fmt, const Args &...args)
        {
            AssemblyLogInfo(L"[WARN]", L"33", tok.line, tok.column, tok.pos, fmt, args...);
        }

        template <typename... Args>
        void Info(int32_t pos, const std::wstring &fmt, const Args &...args)
        {
            auto lineNum = 1;
            for (int32_t i = 0; i < pos; ++i)
                if (Record::mSourceCode[i] == L'\n' || Record::mSourceCode[i] == L'\r')
                    lineNum++;

            AssemblyLogInfo(L"[INFO]", L"32", lineNum, 1, pos, fmt, args...);
        }

        template <typename... Args>
        void Info(Token tok, const std::wstring &fmt, const Args &...args)
        {
            AssemblyLogInfo(L"[INFO]", L"32", tok.line, tok.pos, fmt, args...);
        }
    }

}