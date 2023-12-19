#include "Library.h"
#include "Utils.h"

#include <stdio.h>
#include <ctime>
#include <iostream>
namespace lwscript
{
    LibraryManager &LibraryManager::Instance() noexcept
    {
        static LibraryManager instance;
        return instance;
    }

    LibraryManager::LibraryManager()
    {
        auto ioClass = new ClassObject(L"io");
        auto dsClass = new ClassObject(L"ds");
        auto memClass = new ClassObject(L"mem");
        auto timeClass = new ClassObject(L"time");

#define PRINT_LAMBDA(fn) [](Value *args, uint32_t argCount, const Token *relatedToken, Value &result) -> bool \
{                                                                                                             \
    if (args == nullptr)                                                                                      \
        return false;                                                                                         \
    if (argCount == 1)                                                                                        \
    {                                                                                                         \
        fn(L"{}", args[0].ToString());                                                                        \
        return false;                                                                                         \
    }                                                                                                         \
    if (!IS_STR_VALUE(args[0]))                                                                               \
    {                                                                                                         \
        for (uint32_t i = 0; i < argCount; ++i)                                                               \
            fn(L"{}", args[i].ToString());                                                                    \
        return false;                                                                                         \
    }                                                                                                         \
    std::wstring content = TO_STR_VALUE(args[0])->value;                                                      \
    if (argCount != 1) /*formatting output*/                                                                  \
    {                                                                                                         \
        size_t pos = content.find(L"{}");                                                                     \
        size_t argpos = 1;                                                                                    \
        while (pos != std::wstring::npos)                                                                     \
        {                                                                                                     \
            if (argpos < argCount)                                                                            \
                content.replace(pos, 2, args[argpos++].ToString());                                           \
            else                                                                                              \
                content.replace(pos, 2, L"null");                                                             \
            pos = content.find(L"{}");                                                                        \
        }                                                                                                     \
    }                                                                                                         \
    size_t pos = content.find(L"\\n");                                                                        \
    while (pos != std::wstring::npos)                                                                         \
    {                                                                                                         \
        content[pos] = '\n';                                                                                  \
        content.replace(pos + 1, 1, L""); /*erase a char*/                                                    \
        pos = content.find(L"\\n");                                                                           \
    }                                                                                                         \
    pos = content.find(L"\\t");                                                                               \
    while (pos != std::wstring::npos)                                                                         \
    {                                                                                                         \
        content[pos] = '\t';                                                                                  \
        content.replace(pos + 1, 1, L""); /*erase a char*/                                                    \
        pos = content.find(L"\\t");                                                                           \
    }                                                                                                         \
    pos = content.find(L"\\r");                                                                               \
    while (pos != std::wstring::npos)                                                                         \
    {                                                                                                         \
        content[pos] = '\r';                                                                                  \
        content.replace(pos + 1, 1, L""); /*erase a char*/                                                    \
        pos = content.find(L"\\r");                                                                           \
    }                                                                                                         \
    fn(L"{}", content);                                                                                       \
    return false;                                                                                             \
}

        ioClass->members[L"print"] = new NativeFunctionObject(PRINT_LAMBDA(Print));
        ioClass->members[L"println"] = new NativeFunctionObject(PRINT_LAMBDA(Println));

        dsClass->members[L"sizeof"] = new NativeFunctionObject([](Value *args, uint32_t argCount, const Token *relatedToken, Value &result) -> bool
                                                               {
                                                                   if (args == nullptr || argCount > 1)
                                                                       Hint::Error(relatedToken, L"[Native function 'sizeof']:Expect a argument.");

                                                                   if (IS_ARRAY_VALUE(args[0]))
                                                                   {
                                                                       result = Value((int64_t)TO_ARRAY_VALUE(args[0])->elements.size());
                                                                       return true;
                                                                   }
                                                                   else if (IS_DICT_VALUE(args[0]))
                                                                   {
                                                                       result = Value((int64_t)TO_DICT_VALUE(args[0])->elements.size());
                                                                       return true;
                                                                   }
                                                                   else if (IS_STR_VALUE(args[0]))
                                                                   {
                                                                       result = Value((int64_t)TO_STR_VALUE(args[0])->value.size());
                                                                       return true;
                                                                   }
                                                                   else
                                                                       Hint::Error(relatedToken, L"[Native function 'sizeof']:Expect a array,dict ot string argument.");

                                                                   return false;
                                                               });

        dsClass->members[L"insert"] = new NativeFunctionObject([](Value *args, uint32_t argCount, const Token *relatedToken, Value &result) -> bool
                                                               {
                                                                   if (args == nullptr || argCount != 3)
                                                                       Hint::Error(relatedToken, L"[Native function 'insert']:Expect 3 arguments,the arg0 must be array,dict or string object.The arg1 is the index object.The arg2 is the value object.");

                                                                   if (IS_ARRAY_VALUE(args[0]))
                                                                   {
                                                                       ArrayObject *array = TO_ARRAY_VALUE(args[0]);
                                                                       if (!IS_INT_VALUE(args[1]))
                                                                           Hint::Error(relatedToken, L"[Native function 'insert']:Arg1 must be integer type while insert to a array");

                                                                       int64_t iIndex = TO_INT_VALUE(args[1]);

                                                                       if (iIndex < 0 || iIndex >= (int64_t)array->elements.size())
                                                                           Hint::Error(relatedToken, L"[Native function 'insert']:Index out of array's range");

                                                                       array->elements.insert(array->elements.begin() + iIndex, 1, args[2]);
                                                                   }
                                                                   else if (IS_DICT_VALUE(args[0]))
                                                                   {
                                                                       DictObject *dict = TO_DICT_VALUE(args[0]);

                                                                       for (auto [key, value] : dict->elements)
                                                                           if (key == args[1])
                                                                               Hint::Error(relatedToken, L"[Native function 'insert']:Already exist value in the dict object of arg1" + args[1].ToString());

                                                                       dict->elements[args[1]] = args[2];
                                                                   }
                                                                   else if (IS_STR_VALUE(args[0]))
                                                                   {
                                                                       auto &string = TO_STR_VALUE(args[0])->value;
                                                                       if (!IS_INT_VALUE(args[1]))
                                                                           Hint::Error(relatedToken, L"[Native function 'insert']:Arg1 must be integer type while insert to a array");

                                                                       int64_t iIndex = TO_INT_VALUE(args[1]);

                                                                       if (iIndex < 0 || iIndex >= (int64_t)string.size())
                                                                           Hint::Error(relatedToken, L"[Native function 'insert']:Index out of array's range");

                                                                       string.insert(iIndex, args[2].ToString());
                                                                   }
                                                                   else
                                                                       Hint::Error(relatedToken, L"[Native function 'insert']:Expect a array,dict ot string argument.");

                                                                   result = args[0];
                                                                   return true;
                                                               });

        dsClass->members[L"erase"] = new NativeFunctionObject([](Value *args, uint32_t argCount, const Token *relatedToken, Value &result) -> bool
                                                              {
                                                                  if (args == nullptr || argCount != 2)
                                                                      Hint::Error(relatedToken, L"[Native function 'erase']:Expect 2 arguments,the arg0 must be array,dict or string object.The arg1 is the corresponding index object.");

                                                                  if (IS_ARRAY_VALUE(args[0]))
                                                                  {
                                                                      ArrayObject *array = TO_ARRAY_VALUE(args[0]);
                                                                      if (!IS_INT_VALUE(args[1]))
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:Arg1 must be integer type while insert to a array");

                                                                      int64_t iIndex = TO_INT_VALUE(args[1]);

                                                                      if (iIndex < 0 || iIndex >= (int64_t)array->elements.size())
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:Index out of array's range");

                                                                      array->elements.erase(array->elements.begin() + iIndex);
                                                                  }
                                                                  else if (IS_DICT_VALUE(args[0]))
                                                                  {
                                                                      DictObject *dict = TO_DICT_VALUE(args[0]);

                                                                      bool hasValue = false;

                                                                      for (auto it = dict->elements.begin(); it != dict->elements.end(); ++it)
                                                                          if (it->first == args[1])
                                                                          {
                                                                              dict->elements.erase(it);
                                                                              hasValue = true;
                                                                              break;
                                                                          }

                                                                      if (!hasValue)
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:No corresponding index in dict.");
                                                                  }
                                                                  else if (IS_STR_VALUE(args[0]))
                                                                  {
                                                                      auto &string = TO_STR_VALUE(args[0])->value;
                                                                      if (!IS_INT_VALUE(args[1]))
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:Arg1 must be integer type while insert to a array");

                                                                      int64_t iIndex = TO_INT_VALUE(args[1]);

                                                                      if (iIndex < 0 || iIndex >= (int64_t)string.size())
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:Index out of array's range");

                                                                      string.erase(string.begin() + iIndex);
                                                                  }
                                                                  else
                                                                      Hint::Error(relatedToken, L"[Native function 'erase']:Expect a array,dict ot string argument.");

                                                                  result = args[0];
                                                                  return true;
                                                              });

        memClass->members[L"addressof"] = new NativeFunctionObject([](Value *args, uint32_t argCount, const Token *relatedToken, Value &result) -> bool
                                                                   {
                                                                       if (args == nullptr || argCount != 1)
                                                                           Hint::Error(relatedToken, L"[Native function 'addressof']:Expect 1 arguments.");

                                                                       if (!IS_OBJECT_VALUE(args[0]))
                                                                           Hint::Error(relatedToken, L"[Native function 'addressof']:The arg0 is a value,only object has address.");

                                                                       result = new StrObject(PointerAddressToString(args[0].object));
                                                                       return true;
                                                                   });

        timeClass->members[L"clock"] = new NativeFunctionObject([](Value *args, uint32_t argCount, const Token *relatedToken, Value &result) -> bool
                                                                {
                                                                    result = Value((double)clock() / CLOCKS_PER_SEC);
                                                                    return true;
                                                                });

        mStdLibraries.emplace_back(ioClass);
        mStdLibraries.emplace_back(dsClass);
        mStdLibraries.emplace_back(memClass);
        mStdLibraries.emplace_back(timeClass);
    }
    LibraryManager::~LibraryManager()
    {
    }
}