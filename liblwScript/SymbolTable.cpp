#include "SymbolTable.h"
#include "Utils.h"
namespace lws
{
    SymbolTable::SymbolTable()
        : mSymbolIdx(0)
    {
    }

    Symbol SymbolTable::DefineVariable(const std::wstring &name)
    {
        if (mSymbolIdx >= mSymbols.size())
            ASSERT(L"Too many variables in current scope.");
        for (int16_t i = 0; i < mSymbolIdx; ++i)
        {
            if (mSymbols[mSymbolIdx].name == name)
                ASSERT(L"Redefinition variable:" + name);
        }

        auto *symbol = &mSymbols[mSymbolIdx++];
        symbol->name = name;
        symbol->idx = mSymbolIdx - 1;
        symbol->type = SymbolType::GLOBAL;
        return *symbol;
    }

    Symbol SymbolTable::Resolve(const std::wstring &name)
    {
        for (int16_t i = 0; i < mSymbolIdx; ++i)
            if (mSymbols[i].name == name)
                return mSymbols[i];
        ASSERT(L"No variable:" + name + L" in current scope.");
    }
}