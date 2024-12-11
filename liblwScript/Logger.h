#pragma once
#include <iostream>
#include <sstream>
#include <string_view>
#include <string>
#include <cassert>
#include <cstdarg>
#include "Token.h"
#include "Defines.h"

namespace lwscript
{
    namespace Logger
    {
        namespace Record
        {
            inline STD_STRING mCurFilePath = TEXT("interpreter");
            inline STD_STRING mSourceCode = TEXT("");
        }

        inline void Output(STD_OSTREAM &os, STD_STRING s)
        {
            os << s;
        }

        template <typename T, typename... Args>
        inline void Output(STD_OSTREAM &os, STD_STRING s, const T &next, const Args &...args)
        {
            auto index = s.find_first_of(TEXT("{}"));
            if (index == STD_STRING::npos)
                Output(os, s);
            else
            {
                STD_STRING_STREAM sstr;
                sstr << next;
                s.replace(index, 2, sstr.str());
                sstr.clear();
                Output(os, s, args...);
            }
        }

        template <typename... Args>
        inline void Println(const STD_STRING &s, const Args &...args)
        {
            Output(COUT, s + TEXT("\n"), args...);
        }

        template <typename... Args>
        inline void Print(const STD_STRING &s, const Args &...args)
        {
            Output(COUT, s, args...);
        }

        inline void RecordSource(STD_STRING_VIEW sourceCode)
        {
            Record::mSourceCode = sourceCode;
        }

        template <typename... Args>
        inline void AssemblyLogInfo(const STD_STRING &headerHint, const STD_STRING &colorHint, uint64_t lineNum, uint64_t column, uint64_t pos, const STD_STRING &fmt, const Args &...args)
        {
            auto start = pos;
            auto end = pos;

            if (Record::mCurFilePath != TEXT("interpreter"))
            {
                while ((Record::mSourceCode[start - 1] != TCHAR('\n') && Record::mSourceCode[start - 1] != TCHAR('\r')) && start - 1 > 0)
                    start--;

                while ((Record::mSourceCode[end] != TCHAR('\n') && Record::mSourceCode[end] != TCHAR('\r')) && end < Record::mSourceCode.size())
                    end++;
            }
            else
            {
                start = 0;
                end = Record::mSourceCode.size();
            }

            auto startStr = headerHint + TEXT(":") + Record::mCurFilePath + TEXT("(line ") + TO_STRING(lineNum) + TEXT(",column ") + TO_STRING(column) + TEXT("): ");

            auto lineSrcCode = Record::mSourceCode.substr(start, end - start);

            Println(TEXT("\033[{}m{}{}\033[0m"), colorHint, startStr, lineSrcCode);

            auto blankSize = startStr.size() + pos - start;

            STD_STRING errorHintStr;
            errorHintStr.insert(0, blankSize, TCHAR(' '));
            errorHintStr += TEXT("^ ") + STD_STRING(fmt);

            Println(TEXT("\033[{}m") + errorHintStr + TEXT("\033[0m"), colorHint, args...);
        }

        template <typename... Args>
        inline void Error(const STD_STRING &fmt, const Args &...args)
        {
            Println(TEXT("\033[31m[ERROR]:") + fmt + TEXT("\033[0m"), args...);
#ifndef NDEBUG
            assert(0);
#else
            exit(1);
#endif
        }

        template <typename... Args>
        inline void Error(uint64_t pos, const STD_STRING &fmt, const Args &...args)
        {
            auto lineNum = 1;
            for (uint64_t i = 0; i < pos; ++i)
                if (Record::mSourceCode[i] == TCHAR('\n') || Record::mSourceCode[i] == TCHAR('\r'))
                    lineNum++;

            AssemblyLogInfo(TEXT("[ERROR]"), TEXT("31"), lineNum, 1, pos, fmt, args...);
#ifndef NDEBUG
            assert(0);
#else
            exit(1);
#endif
        }

        template <typename... Args>
        inline void Error(const Token *tok, const STD_STRING &fmt, const Args &...args)
        {
            AssemblyLogInfo(TEXT("[ERROR]"), TEXT("31"), tok->sourceLocation.line, tok->sourceLocation.column, tok->sourceLocation.pos, fmt, args...);
#ifndef NDEBUG
            assert(0);
#else
            exit(1);
#endif
        }

        template <typename... Args>
        inline void Warn(const STD_STRING &fmt, const Args &...args)
        {
            Println(TEXT("\033[33m[WARN]:") + fmt + TEXT("\033[0m"), args...);
        }

        template <typename... Args>
        void Warn(int32_t pos, const STD_STRING &fmt, const Args &...args)
        {
            auto lineNum = 1;
            for (int32_t i = 0; i < pos; ++i)
                if (Record::mSourceCode[i] == TCHAR('\n') || Record::mSourceCode[i] == TCHAR('\r'))
                    lineNum++;

            AssemblyLogInfo(TEXT("[WARN]"), TEXT("33"), lineNum, 1, pos, fmt, args...);
        }

        template <typename... Args>
        void Warn(const Token *tok, const STD_STRING &fmt, const Args &...args)
        {
            AssemblyLogInfo(TEXT("[WARN]"), TEXT("33"), tok->sourceLocation.line, tok->sourceLocation.column, tok->sourceLocation.pos, fmt, args...);
        }

        template <typename... Args>
        inline void Info(const STD_STRING &fmt, const Args &...args)
        {
            Println(TEXT("\033[32m[INFO]:") + fmt + TEXT("\033[0m"), args...);
        }

        template <typename... Args>
        void Info(int32_t pos, const STD_STRING &fmt, const Args &...args)
        {
            auto lineNum = 1;
            for (int32_t i = 0; i < pos; ++i)
                if (Record::mSourceCode[i] == TCHAR('\n') || Record::mSourceCode[i] == TCHAR('\r'))
                    lineNum++;

            AssemblyLogInfo(TEXT("[INFO]"), TEXT("32"), lineNum, 1, pos, fmt, args...);
        }

        template <typename... Args>
        void Info(const Token *tok, const STD_STRING &fmt, const Args &...args)
        {
            AssemblyLogInfo(TEXT("[INFO]"), TEXT("32"), tok->sourceLocation.line, tok->sourceLocation.column, tok->sourceLocation.pos, fmt, args...);
        }
    }
}