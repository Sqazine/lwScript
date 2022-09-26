#include "SymbolTable.h"
#include "Utils.h"
namespace lws
{
    SymbolTable::SymbolTable()
        : mSymbolIdx(0)
    {
    }
    uint8_t SymbolTable::DeclareVariable(const std::wstring& name)
    {
        if(mSymbolIdx>=mSymbols.size())
            ASSERT(L"Too many variables in current scope.");
        for(int16_t i=0;i<mSymbolIdx;++i)
        {
            if(mSymbols[mSymbolIdx].name==name)
                ASSERT(L"Redefinition variable:"+name);
        }

        mSymbols[mSymbolIdx++]=Symbol(name,SymbolType::GLOBAL);
        return mSymbolIdx-1;
    }

    Symbol SymbolTable::DefineVariable(uint8_t idx)
    {
        mSymbols[idx].type=SymbolType::GLOBAL;
        return mSymbols[idx];
    }
}