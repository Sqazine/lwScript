#pragma once
#include <string_view>
#include <unordered_map>
#include "Value.h"
namespace lws
{
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

    class Context
    {
    public:
        Context();
        Context(Context *upContext);
        ~Context();

        void DefineVariableByName(std::wstring_view name, ValueDescType objDescType, const Value &value);
        void DefineVariableByName(std::wstring_view name, const ValueDesc &objectDesc);
        void AssignVariableByName(std::wstring_view name, const Value &value);
        Value GetVariableByName(std::wstring_view name);

        void AssignVariableByAddress(std::wstring_view address, const Value &value);
        Value GetVariableByAddress(std::wstring_view address);

        Context *GetUpContext();
        void SetUpContext(Context *env);

        Context *GetRoot();

    private:
        friend class VM;
        friend struct ClassObject;

        std::unordered_map<std::wstring, ValueDesc> mValues;
        Context *mUpContext;
    };
}