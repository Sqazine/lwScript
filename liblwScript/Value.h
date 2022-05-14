#pragma once
#include <string>
#include <unordered_map>
namespace lws
{
#define IS_NULL_VALUE(v) (v.Type() == VALUE_NULL)
#define IS_INT_VALUE(v) (v.Type() == VALUE_INT)
#define IS_REAL_VALUE(v) (v.Type() == VALUE_REAL)
#define IS_BOOL_VALUE(v) (v.Type() == VALUE_BOOL)
#define IS_STR_VALUE(v) (v.Type()==VALUE_STR)
#define IS_OBJECT_VALUE(v) (v.Type() == VALUE_OBJECT)
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ARRAY_OBJ(v.object))
#define IS_TABLE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_TABLE_OBJ(v.object))
#define IS_FIELD_VALUE(v) (IS_OBJECT_VALUE(v) && IS_FIELD_OBJ(v.object))
#define IS_REF_VALUE(v) (IS_OBJECT_VALUE(v) && IS_REF_OBJ(v.object))
#define IS_LAMBDA_VALUE(v) (IS_OBJECT_VALUE(v) && IS_LAMBDA_OBJ(v.object))
#define IS_INVALID_VALUE(v) (v == gInvalidValue)

#define TO_INT_VALUE(v) (v.integer)
#define TO_REAL_VALUE(v) (v.realnum)
#define TO_BOOL_VALUE(v) (v.boolean)
#define TO_STR_VALUE(v) (v.string)
#define TO_OBJECT_VALUE(v) (v.object)
#define TO_ARRAY_VALUE(v) (TO_ARRAY_OBJ(v.object))
#define TO_TABLE_VALUE(v) (TO_TABLE_OBJ(v.object))
#define TO_LAMBDA_VALUE(v) (TO_LAMBDA_OBJ(v.object))
#define TO_FIELD_VALUE(v) (TO_FIELD_OBJ(v.object))
#define TO_REF_VALUE(v) (TO_REF_OBJ(v.object))

    enum ValueType
    {
        VALUE_NULL,
        VALUE_INT,
        VALUE_REAL,
        VALUE_BOOL,
        VALUE_STR,
        VALUE_OBJECT,
        VALUE_INVALID,
    };

    struct Value
    {
        Value();
        Value(int64_t integer);
        Value(double number);
        Value(bool boolean);
        Value(const std::wstring& string);
        Value(struct Object *object);
        Value(ValueType type);
        Value(const Value &v);
        ~Value();

        Value &operator=(const Value &v);

        ValueType Type() const;
        std::wstring Stringify() const;
        void Mark() const;
        void UnMark() const;

        ValueType type;

        union
        {
            int64_t integer;
            double realnum;
            bool boolean;
            std::wstring string=L"";
            struct Object *object;
        };
    };

    const Value gInvalidValue = Value(VALUE_INVALID);

    bool operator==(const Value &left, const Value &right);
    bool operator!=(const Value &left, const Value &right);

    struct ValueHash
    {
        size_t operator()(const Value &v) const;
    };

    typedef std::unordered_map<Value, Value, ValueHash> ValueUnorderedMap;
}