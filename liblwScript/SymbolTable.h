#pragma once
#include <string>
#include <array>
#include "Config.h"
#include "Value.h"
namespace lws
{
    enum SymbolType
    {
        SYMBOL_GLOBAL,
        SYMBOL_LOCAL,
    };

    struct Symbol
    {
        std::wstring name;
        SymbolType type = SYMBOL_GLOBAL;
        ValueDesc descType = DESC_VARIABLE;
        uint8_t location = 0;
        uint8_t index = 0;
        int8_t depth = -1;
    };
    class SymbolTable
    {
    public:
        SymbolTable();
        SymbolTable(SymbolTable *enclosing);

        Symbol Define(ValueDesc descType, const std::wstring &name);

        Symbol Resolve(const std::wstring &name);

        std::array<Symbol, UINT8_COUNT> mSymbols;
        uint8_t mSymbolCount;
        uint8_t mGlobalSymbolCount;
        uint8_t mLocalSymbolCount;
        uint8_t mScopeDepth;
        SymbolTable *enclosing;
    };
}
