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
        Symbol() {}
        std::wstring name;
        SymbolType type = SymbolType::GLOBAL;
        SymbolDescType descType = SymbolDescType::VARIABLE;
        uint8_t idx = 0;
        int8_t depth = -1;
    };
    class SymbolTable
    {
    public:
        SymbolTable();
        SymbolTable(SymbolTable *enclosing);

        uint8_t Declare(SymbolDescType descType, const std::wstring &name);

        Symbol Define(uint8_t idx);

        Symbol Resolve(const std::wstring &name);

        std::array<Symbol, UINT8_COUNT> mSymbols;
        uint8_t mSymbolIdx;
        uint8_t mScopeDepth;
        SymbolTable *enclosing;
    };
}
