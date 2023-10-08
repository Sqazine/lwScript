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

    Symbol SymbolTable::Define(const Token* relatedToken, ValueDesc descType, const std::wstring &name, const FunctionSymbolInfo &functionInfo)
    {
        if (mSymbolCount >= mSymbols.size())
            Hint::Error(relatedToken, L"Too many symbols in current scope.");
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            auto isSameParamCount = (mSymbols[i].functionSymInfo.paramCount < 0 || functionInfo.paramCount < 0) ? true : mSymbols[i].functionSymInfo.paramCount == functionInfo.paramCount;
            if (mSymbols[i].scopeDepth == -1 || mSymbols[i].scopeDepth < mScopeDepth)
                break;
            if (mSymbols[i].name == name && isSameParamCount)
                Hint::Error(relatedToken, L"Redefinition symbol:{}", name);
        }

        auto *symbol = &mSymbols[mSymbolCount++];
        symbol->name = name;
        symbol->descType = descType;
        symbol->functionSymInfo = functionInfo;
        symbol->relatedToken = relatedToken;

        if (mScopeDepth == 0)
        {
            symbol->type = SymbolType::GLOBAL;
            symbol->index = mGlobalSymbolCount++;
        }
        else
        {
            symbol->type = SymbolType::LOCAL;
            symbol->index = mLocalSymbolCount++;
        }
        symbol->scopeDepth = mScopeDepth;
        return *symbol;
    }

    Symbol SymbolTable::Resolve(const Token* relatedToken, const std::wstring &name, int8_t paramCount, int8_t d)
    {
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            auto isSameParamCount = (mSymbols[i].functionSymInfo.paramCount < 0 || paramCount < 0) ? true : mSymbols[i].functionSymInfo.paramCount == paramCount;

            if (mSymbols[i].name == name && mSymbols[i].scopeDepth <= mScopeDepth)
            {
                if (isSameParamCount || mSymbols[i].functionSymInfo.varArgParamType > 0)
                {
                    if (mSymbols[i].scopeDepth == -1)
                        Hint::Error(relatedToken, L"symbol not defined yet!");

                    if (d == 1)
                        mSymbols[i].isCaptured = true;

                    return mSymbols[i];
                }
            }
        }

        if (enclosing)
        {
            Symbol result = enclosing->Resolve(relatedToken, name, paramCount, ++d);
            if (d > 0 && result.type != SymbolType::GLOBAL)
            {
                result.type = SymbolType::UPVALUE;
                result.upvalue = AddUpValue(relatedToken, result.index, enclosing->mTableDepth);
            }
            return result;
        }

        Hint::Error(relatedToken, L"No symbol: \"{}\" in current scope.", name);
    }

    UpValue SymbolTable::AddUpValue(const Token* relatedToken, uint8_t location, uint8_t depth)
    {
        for (int32_t i = 0; i < mUpValueCount; ++i)
        {
            UpValue *upvalue = &mUpValues[i];
            if (upvalue->location == location && upvalue->depth == depth)
                return *upvalue;
        }

        if (mUpValueCount == UINT8_COUNT)
            Hint::Error(relatedToken, L"Too many closure upvalues in function.");
        mUpValues[mUpValueCount].location = location;
        mUpValues[mUpValueCount].depth = depth;
        mUpValues[mUpValueCount].index = mUpValueCount;
        mUpValueCount++;
        return mUpValues[mUpValueCount - 1];
    }
}