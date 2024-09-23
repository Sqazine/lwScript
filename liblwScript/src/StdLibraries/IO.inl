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

auto ioClass = new ClassObject(L"io");
ioClass->members[L"print"] = new NativeFunctionObject(PRINT_LAMBDA(Print));
ioClass->members[L"println"] = new NativeFunctionObject(PRINT_LAMBDA(Println));

mLibraries.emplace_back(ioClass);