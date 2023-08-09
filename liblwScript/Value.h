#pragma once
#include <string>
#include <unordered_map>
namespace lws
{
    enum ValueType
    {
        VALUE_NULL,
        VALUE_INT,
        VALUE_REAL,
        VALUE_BOOL,
        VALUE_OBJECT,
    };

    enum class ValueDesc
    {
        VARIABLE,
        CONSTANT
    };

    struct Value
    {
        Value();
        Value(int64_t integer);
        Value(double number);
        Value(bool boolean);
        Value(struct Object *object);
        ~Value();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const;
        void Mark(class VM *vm) const;
        void UnMark() const;
        Value Clone() const;

        ValueType type;
        ValueDesc desc = ValueDesc::VARIABLE;

        union
        {
            int64_t integer;
            double realnum;
            bool boolean;
            struct Object *object;
        };
    };

    bool operator==(const Value &left, const Value &right);
    bool operator!=(const Value &left, const Value &right);
}