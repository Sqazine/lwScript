#pragma once
#include <string>
#include <array>
#include "Config.h"
namespace lws
{
    enum class SymbolType
    {
        GLOBAL,
        LOCAL,
    };

    enum class SymbolDescType
    {
        VARIABLE,
        CONSTANT,
    };

    struct Symbol
    {
        std::wstring name;
        SymbolType type = SymbolType::GLOBAL;
        SymbolDescType descType = SymbolDescType::VARIABLE;
        uint8_t location = 0;
        uint8_t index = 0;
        int8_t depth = -1;
    };
    class SymbolTable
    {
    public:
        SymbolTable();
        SymbolTable(SymbolTable *enclosing);

        Symbol Define(SymbolDescType descType, const std::wstring &name);

        Symbol Resolve(const std::wstring &name);

        std::array<Symbol, UINT8_COUNT> mSymbols;
        uint8_t mSymbolCount;
        uint8_t mGlobalSymbolCount;
        uint8_t mLocalSymbolCount;
        uint8_t mScopeDepth;
        SymbolTable *enclosing;
    };
}
