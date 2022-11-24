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
        SYMBOL_UPVALUE,
    };

    struct UpValue
    {
        uint8_t index = 0;
        uint8_t location=0;
        uint8_t depth = -1;
    };

    struct Symbol
    {
        std::wstring name;
        SymbolType type = SYMBOL_GLOBAL;
        ValueDesc descType = DESC_VARIABLE;
        uint8_t index = 0;
        int8_t scopeDepth = -1;
        int8_t paramCount = -1;
        UpValue upvalue;//available only while type is SYMBOL_UPVALUE
        bool isCaptured=false;  
    };
    class SymbolTable
    {
    public:
        SymbolTable();
        SymbolTable(SymbolTable *enclosing);

        Symbol Define(ValueDesc descType, const std::wstring &name, int8_t paramCount = -1);

        Symbol Resolve(const std::wstring &name, int8_t paramCount = -1, int8_t d = 0);

        std::array<Symbol, UINT8_COUNT> mSymbols;
        uint8_t mSymbolCount;
        uint8_t mGlobalSymbolCount;
        uint8_t mLocalSymbolCount;
        std::array<UpValue, UINT8_COUNT> mUpValues;
        int32_t mUpValueCount;
        uint8_t mScopeDepth; //Depth of scope nesting(related to code {} scope)
        uint8_t mTableDepth; //Depth of symbol table nesting(related to symboltable's enclosing)
        SymbolTable *enclosing;

    private:
        UpValue AddUpValue(uint8_t location, uint8_t depth);
    };
}
