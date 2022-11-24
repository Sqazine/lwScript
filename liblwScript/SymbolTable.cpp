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

    Symbol SymbolTable::Define(ValueDesc descType, const std::wstring &name, int8_t paramCount)
    {
        if (mSymbolCount >= mSymbols.size())
            ASSERT(L"Too many symbols in current scope.");
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            auto isSameParamCount = (mSymbols[i].paramCount < 0 || paramCount < 0) ? true : mSymbols[i].paramCount == paramCount;
            if (mSymbols[i].name == name && isSameParamCount)
                ASSERT(L"Redefinition symbol:" + name);
            if (mSymbols[i].depth == -1 && mSymbols[i].depth < mScopeDepth)
                break;
        }

        auto *symbol = &mSymbols[mSymbolCount++];
        symbol->name = name;
        symbol->descType = descType;
        symbol->paramCount = paramCount;

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
        symbol->depth = mScopeDepth;
        return *symbol;
    }

    Symbol SymbolTable::Resolve(const std::wstring &name, int8_t paramCount)
    {
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            auto isSameParamCount = (mSymbols[i].paramCount < 0 || paramCount < 0) ? true : mSymbols[i].paramCount == paramCount;
            if (mSymbols[i].name == name && isSameParamCount && mSymbols[i].depth <= mScopeDepth)
            {
                if (mSymbols[i].depth == -1)
                    ASSERT("symbol not defined yet!");
                return mSymbols[i];
            }
        }

        if (enclosing)
            return enclosing->Resolve(name, paramCount);

        ASSERT(L"No symbol:" + name + L" in current scope.");
    }
}