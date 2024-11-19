#include "SymbolTable.h"
#include "Utils.h"
#include "Logger.h"
namespace lwscript
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

    Symbol SymbolTable::Define(const Token *relatedToken, Privilege privilege, const STD_STRING &name, const FunctionSymbolInfo &functionInfo)
    {
        if (mSymbolCount >= mSymbols.size())
            Logger::Error(relatedToken, TEXT("Too many symbols in current scope."));
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            auto isSameParamCount = (mSymbols[i].functionSymInfo.paramCount < 0 || functionInfo.paramCount < 0) ? true : mSymbols[i].functionSymInfo.paramCount == functionInfo.paramCount;
            if (mSymbols[i].scopeDepth == -1 || mSymbols[i].scopeDepth < mScopeDepth)
                break;
            if (mSymbols[i].name == name && isSameParamCount)
                Logger::Error(relatedToken, TEXT("Redefinition symbol:{}"), name);
        }

        auto *symbol = &mSymbols[mSymbolCount++];
        symbol->name = name;
        symbol->privilege = privilege;
        symbol->functionSymInfo = functionInfo;
        symbol->relatedToken = relatedToken;

        if (mScopeDepth == 0)
        {
            symbol->kind = SymbolKind::GLOBAL;
            symbol->index = mGlobalSymbolCount++;
        }
        else
        {
            symbol->kind = SymbolKind::LOCAL;
            symbol->index = mLocalSymbolCount++;
        }
        symbol->scopeDepth = mScopeDepth;
        return *symbol;
    }

    Symbol SymbolTable::Resolve(const Token *relatedToken, const STD_STRING &name, int8_t paramCount, int8_t d)
    {
        for (int16_t i = mSymbolCount - 1; i >= 0; --i)
        {
            auto isSameParamCount = (mSymbols[i].functionSymInfo.paramCount < 0 || paramCount < 0) ? true : mSymbols[i].functionSymInfo.paramCount == paramCount;

            if (mSymbols[i].name == name && mSymbols[i].scopeDepth <= mScopeDepth)
            {
                if (isSameParamCount || mSymbols[i].functionSymInfo.varArg > VarArg::NONE)
                {
                    if (mSymbols[i].scopeDepth == -1)
                        Logger::Error(relatedToken, TEXT("symbol not defined yet!"));

                    if (d == 1)
                        mSymbols[i].isCaptured = true;

                    return mSymbols[i];
                }
            }
        }

        if (enclosing)
        {
            Symbol result = enclosing->Resolve(relatedToken, name, paramCount, ++d);
            if (d > 0 && result.kind != SymbolKind::GLOBAL)
            {
                result.kind = SymbolKind::UPVALUE;
                result.upvalue = AddUpValue(relatedToken, result.index, enclosing->mTableDepth);
            }
            return result;
        }

        Logger::Error(relatedToken, TEXT("No symbol: \"{}\" in current scope."), name);
    }

    UpValue SymbolTable::AddUpValue(const Token *relatedToken, uint8_t location, uint8_t depth)
    {
        for (int32_t i = 0; i < mUpValueCount; ++i)
        {
            UpValue *upvalue = &mUpValues[i];
            if (upvalue->location == location && upvalue->depth == depth)
                return *upvalue;
        }

        if (mUpValueCount == UINT8_COUNT)
            Logger::Error(relatedToken, TEXT("Too many closure upvalues in function."));
        mUpValues[mUpValueCount].location = location;
        mUpValues[mUpValueCount].depth = depth;
        mUpValues[mUpValueCount].index = mUpValueCount;
        mUpValueCount++;
        return mUpValues[mUpValueCount - 1];
    }
}