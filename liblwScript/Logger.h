#pragma once
#include <iostream>
#include <sstream>
#include <string_view>
#include <string>
#include <cassert>
#include <cstdarg>
#include "Token.h"

namespace lwscript
{
    inline void Output(std::wostream &os, std::wstring s)
    {
        os << s;
    }

    template <typename T, typename... Args>
    inline void Output(std::wostream &os, std::wstring s, const T &next, const Args &...args)
    {
        auto index = s.find_first_of(L"{}");
        if (index == std::wstring::npos)
            Output(os, s);
        else
        {
            std::wstringstream sstr;
            sstr << next;
            s.replace(index, 2, sstr.str());
            sstr.clear();
            Output(os, s, args...);
        }
    }

    inline void Output(std::ostream &os, std::string_view s)
    {
        os << s;
    }

    template <typename T, typename... Args>
    inline void Output(std::ostream &os, std::string_view s, const T &next, const Args &...args)
    {
        auto index = s.find_first_of("{}");
        if (index == std::string::npos)
            Output(os, s);
        else
        {
            std::string tmpS = s.data();
            std::stringstream sstr;
            sstr << next;
            tmpS.replace(index, 2, sstr.str());
            sstr.clear();
            Output(os, tmpS, args...);
        }
    }

    template <typename... Args>
    inline void Println(const std::wstring &s, const Args &...args)
    {
        Output(std::wcout, s + L"\n", args...);
    }

    template <typename... Args>
    inline void Print(const std::wstring &s, const Args &...args)
    {
        Output(std::wcout, s, args...);
    }

    template <typename... Args>
    inline void Println(const std::string &s, const Args &...args)
    {
        Output(std::cout, s + "\n", args...);
    }

    template <typename... Args>
    inline void Print(const std::string s, const Args &...args)
    {
        Output(std::cout, s, args...);
    }

    namespace Logger
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
        inline void Error(const std::string &fmt, const Args &...args)
        {
            Println("\033[31m" + fmt + "\033[0m", args...);
#ifndef NDEBUG
            assert(0);
#else
            exit(1);
#endif
        }

        template <typename... Args>
        inline void Error(const std::wstring &fmt, const Args &...args)
        {
            Println(L"\033[31m" + fmt + L"\033[0m", args...);
#ifndef NDEBUG
            assert(0);
#else
            exit(1);
#endif
        }

        template <typename... Args>
        inline void Error(uint64_t pos, const std::wstring &fmt, const Args &...args)
        {
            auto lineNum = 1;
            for (uint64_t i = 0; i < pos; ++i)
                if (Record::mSourceCode[i] == L'\n' || Record::mSourceCode[i] == L'\r')
                    lineNum++;

            AssemblyLogInfo(L"[ERROR]", L"31", lineNum, 1, pos, fmt, args...);
#ifndef NDEBUG
            assert(0);
#else
            exit(1);
#endif
        }

        template <typename... Args>
        inline void Error(const Token *tok, const std::wstring &fmt, const Args &...args)
        {
            AssemblyLogInfo(L"[ERROR]", L"31", tok->line, tok->column, tok->pos, fmt, args...);
#ifndef NDEBUG
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
        void Warn(const Token *tok, const std::wstring &fmt, const Args &...args)
        {
            AssemblyLogInfo(L"[WARN]", L"33", tok->line, tok->column, tok->pos, fmt, args...);
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
        void Info(const Token *tok, const std::wstring &fmt, const Args &...args)
        {
            AssemblyLogInfo(L"[INFO]", L"32", tok->line, tok->column, tok->pos, fmt, args...);
        }
    }
}