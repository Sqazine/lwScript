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
    struct Symbol
    {
        Symbol() {}
        Symbol(std::wstring_view name, SymbolType type) : name(name), type(type) {}
        std::wstring name;
        SymbolType type;
    };
    class SymbolTable
    {
    public:
        SymbolTable();
        uint8_t DeclareVariable(const std::wstring &name);
        Symbol DefineVariable(uint8_t idx);
    private:
        std::array<Symbol, UINT8_COUNT> mSymbols;
        uint8_t mSymbolIdx;
    };
}
