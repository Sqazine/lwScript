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
        SymbolType type;
        SymbolDescType descType;
        uint8_t idx;
    };
    class SymbolTable
    {
    public:
        SymbolTable();
        Symbol Define(SymbolDescType descType, const std::wstring &name);

        Symbol Resolve(const std::wstring& name);
    private:
        std::array<Symbol, UINT8_COUNT> mSymbols;
        uint8_t mSymbolIdx;
    };
}
