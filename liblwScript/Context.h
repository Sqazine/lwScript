#pragma once
#include <string_view>
#include <unordered_map>
#include "Value.h"
namespace lws
{
#define STACK_INCREMENT_RATE 2
#define SCOPE_STACK_MAX 256

    enum class ValueDescType
    {
        CONST,
        VARIABLE
    };

    struct ValueDesc
    {
        ValueDescType type;
        Value value;
    };

    inline bool operator==(const ValueDesc &left, const ValueDesc &right)
    {
        return left.type == right.type && left.value == right.value;
    }

    class Scope
    {
    public:
        Scope();
        ~Scope();

        void DefineVariableByName(std::wstring_view name, ValueDescType objDescType, const Value &value);
        void DefineVariableByName(std::wstring_view name, const ValueDesc &objectDesc);
        bool AssignVariableByName(std::wstring_view name, const Value &value);
        Value GetVariableByName(std::wstring_view name);

        bool AssignVariableByAddress(std::wstring_view address, const Value &value);
        Value GetVariableByAddress(std::wstring_view address);

        void Mark();

        void Reset();

    private:
        friend class VM;

        std::unordered_map<std::wstring, ValueDesc> mValues;
    };

    class Context
    {
    public:
        Context();
        ~Context();

        void DefineVariableByName(std::wstring_view name, ValueDescType objDescType, const Value &value);
        void DefineVariableByName(std::wstring_view name, const ValueDesc &objectDesc);
        void AssignVariableByName(std::wstring_view name, const Value &value);
        Value GetVariableByName(std::wstring_view name);

        void AssignVariableByAddress(std::wstring_view address, const Value &value);
        Value GetVariableByAddress(std::wstring_view address);

        Scope &GetUpScope();

        Scope &GetRootScope();

        void EnterScope();
        void ExitScope();

        void Mark();

    private:
        friend class VM;

        Scope& CurScope();

        size_t mCurScopeDepth;
        std::vector<Scope> mScopeStack;
    };
}