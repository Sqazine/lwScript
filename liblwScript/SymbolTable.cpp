#include "SymbolTable.h"
#include "Utils.h"
namespace lws
{
    SymbolTable::SymbolTable()
        : mSymbolIdx(0), enclosing(nullptr), mScopeDepth(0),mGlobalCount(0),mLocalCount(0)
    {
    }

    SymbolTable::SymbolTable(SymbolTable *enclosing)
        : mSymbolIdx(0), enclosing(enclosing),mGlobalCount(0),mLocalCount(0)
    {
        mScopeDepth += enclosing->mScopeDepth + 1;
    }

    uint8_t SymbolTable::Declare(SymbolDescType descType, const std::wstring &name)
    {
        if (mSymbolIdx >= mSymbols.size())
            ASSERT(L"Too many variables in current scope.");
        for (int16_t i = mSymbolIdx - 1; i >= 0; --i)
        {
            if (mSymbols[mSymbolIdx].depth == -1 && mSymbols[mSymbolIdx].depth < mScopeDepth)
                break;
            if (mSymbols[mSymbolIdx].name == name)
                ASSERT(L"Redefinition variable:" + name);
        }
        auto *symbol = &mSymbols[mSymbolIdx++];
        symbol->name = name;
        symbol->descType = descType;
        symbol->depth = -1;

        return mSymbolIdx-1;
    }

    Symbol SymbolTable::Define(uint8_t idx)
    {

        auto symbol = &mSymbols[idx];
        if (mScopeDepth == 0)
        {
            symbol->type = SymbolType::GLOBAL;
            symbol->idx=mGlobalCount++;
        }
        else
        {
            symbol->type = SymbolType::LOCAL;
            symbol->idx=mLocalCount++;
        }
        symbol->depth = mScopeDepth;
        return *symbol;
    }

    Symbol SymbolTable::Resolve(const std::wstring &name)
    {
        if (!enclosing)
        {
            for (int16_t i = mSymbolIdx - 1; i >= 0; --i)
                if (mSymbols[i].name == name && mSymbols[i].depth <= mScopeDepth)
                {
                    if (mSymbols[i].depth == -1)
                        ASSERT("variable not defined yet!");
                    return mSymbols[i];
                }

            ASSERT(L"No variable:" + name + L" in current scope.");
        }
        else
        {
            return enclosing->Resolve(name);
        }
    }
}