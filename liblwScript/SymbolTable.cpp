#include "SymbolTable.h"
#include "Utils.h"
namespace lws
{
    SymbolTable::SymbolTable()
        : mSymbolCount(0), mGlobalSymbolCount(0), mLocalSymbolCount(0), mUpValueCount(0), enclosing(nullptr), mScopeDepth(0), mTableDepth(0)
    {
    }

    SymbolTable::SymbolTable(SymbolTable *enclosing)
        : mSymbolCount(0), mGlobalSymbolCount(0), mLocalSymbolCount(0), mUpValueCount(0), enclosing(enclosing)
    {
        mScopeDepth = enclosing->mScopeDepth + 1;
        mTableDepth = enclosing->mTableDepth + 1;
    }

    Symbol SymbolTable::Define(ValueDesc descType, const std::wstring &name, int8_t paramCount)
    {
        if (mSymbolCount >= mSymbols.size())
            ASSERT(L"Too many symbols in current scope.")
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            auto isSameParamCount = (mSymbols[i].paramCount < 0 || paramCount < 0) ? true : mSymbols[i].paramCount == paramCount;
            if (mSymbols[i].name == name && isSameParamCount)
                ASSERT(L"Redefinition symbol:" + name)
            if (mSymbols[i].scopeDepth == -1 && mSymbols[i].scopeDepth < mScopeDepth)
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
        symbol->scopeDepth = mScopeDepth;
        return *symbol;
    }

    Symbol SymbolTable::Resolve(const std::wstring &name, int8_t paramCount, int8_t d)
    {
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            auto isSameParamCount = (mSymbols[i].paramCount < 0 || paramCount < 0) ? true : mSymbols[i].paramCount == paramCount;
            if (mSymbols[i].name == name && isSameParamCount && mSymbols[i].scopeDepth <= mScopeDepth)
            {
                if (mSymbols[i].scopeDepth == -1)
                    ASSERT("symbol not defined yet!")

                if (d == 1)
                    mSymbols[i].isCaptured = true;

                return mSymbols[i];
            }
        }

        if (enclosing)
        {
            Symbol result = enclosing->Resolve(name, paramCount, ++d);
            if (d > 0 && result.type != SYMBOL_GLOBAL)
            {
                result.type = SYMBOL_UPVALUE;
                result.upvalue = AddUpValue(result.index, enclosing->mTableDepth);
            }
            return result;
        }

        ASSERT(L"No symbol:" + name + L" in current scope.")
    }

    UpValue SymbolTable::AddUpValue(uint8_t location, uint8_t depth)
    {
        for (int32_t i = 0; i < mUpValueCount; ++i)
        {
            UpValue *upvalue = &mUpValues[i];
            if (upvalue->location == location && upvalue->depth == depth)
                return *upvalue;
        }

        if (mUpValueCount == UINT8_COUNT)
            ASSERT("Too many closure upvalues in function.")
        mUpValues[mUpValueCount].location = location;
        mUpValues[mUpValueCount].depth = depth;
        mUpValues[mUpValueCount].index = mUpValueCount;
        mUpValueCount++;
        return mUpValues[mUpValueCount - 1];
    }
}