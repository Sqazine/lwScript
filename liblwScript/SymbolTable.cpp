#include "SymbolTable.h"
#include "Utils.h"
namespace lws
{
    SymbolTable::SymbolTable()
        : mSymbolCount(0), mGlobalSymbolCount(0), mLocalSymbolCount(0), enclosing(nullptr), mScopeDepth(0)
    {
    }

    SymbolTable::SymbolTable(SymbolTable *enclosing)
        : mSymbolCount(0), mGlobalSymbolCount(0), mLocalSymbolCount(0), enclosing(enclosing)
    {
        mScopeDepth = enclosing->mScopeDepth + 1;
    }

    Symbol SymbolTable::Define(ValueDesc descType, const std::wstring &name)
    {
        if (mSymbolCount >= mSymbols.size())
            ASSERT(L"Too many variables in current scope.");
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            if (mSymbols[mSymbolCount].name == name)
                ASSERT(L"Redefinition variable:" + name);
            if (mSymbols[mSymbolCount].depth == -1 && mSymbols[mSymbolCount].depth < mScopeDepth)
                break;
        }

        auto *symbol = &mSymbols[mSymbolCount++];
        symbol->name = name;
        symbol->descType = descType;

        if (mScopeDepth == 0)
        {
            symbol->type = SYMBOL_GLOBAL;
            symbol->index = mGlobalSymbolCount++;
        }
        else
        {
            symbol->type = SYMBOL_LOCAL;
            symbol->index = mLocalSymbolCount++;
        }
        symbol->location = mSymbolCount - 1;
        symbol->depth = mScopeDepth;
        return *symbol;
    }

    Symbol SymbolTable::Resolve(const std::wstring &name)
    {

        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
            if (mSymbols[i].name == name && mSymbols[i].depth <= mScopeDepth)
            {
                if (mSymbols[i].depth == -1)
                    ASSERT("variable not defined yet!");
                return mSymbols[i];
            }

        if (enclosing)
            return enclosing->Resolve(name);

        ASSERT(L"No variable:" + name + L" in current scope.");
    }
}