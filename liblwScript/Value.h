#pragma once
#include <string>
#include <unordered_map>
#include "Object.h"
namespace lws
{
#define IS_NULL_VALUE(v) (v.Type() == VALUE_NULL)
#define IS_INT_VALUE(v) (v.Type() == VALUE_INT)
#define IS_REAL_VALUE(v) (v.Type() == VALUE_REAL)
#define IS_BOOL_VALUE(v) (v.Type() == VALUE_BOOL)
#define IS_OBJECT_VALUE(v) (v.Type() == VALUE_OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STR_OBJ(v.object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ARRAY_OBJ(v.object))
#define IS_TABLE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_TABLE_OBJ(v.object))
#define IS_CLASS_VALUE(v) (IS_OBJECT_VALUE(v) && IS_CLASS_OBJ(v.object))
#define IS_REF_VALUE(v) (IS_OBJECT_VALUE(v) && IS_REF_OBJ(v.object))
#define IS_LAMBDA_VALUE(v) (IS_OBJECT_VALUE(v) && IS_LAMBDA_OBJ(v.object))

#define TO_INT_VALUE(v) (v.integer)
#define TO_REAL_VALUE(v) (v.realnum)
#define TO_BOOL_VALUE(v) (v.boolean)
#define TO_OBJECT_VALUE(v) (v.object)
#define TO_STR_VALUE(v) (TO_STR_OBJ(v.object)->value)
#define TO_ARRAY_VALUE(v) (TO_ARRAY_OBJ(v.object))
#define TO_TABLE_VALUE(v) (TO_TABLE_OBJ(v.object))
#define TO_LAMBDA_VALUE(v) (TO_LAMBDA_OBJ(v.object))
#define TO_CLASS_VALUE(v) (TO_CLASS_OBJ(v.object))
#define TO_REF_VALUE(v) (TO_REF_OBJ(v.object))

    enum ValueType
    {
        VALUE_NULL,
        VALUE_INT,
        VALUE_REAL,
        VALUE_BOOL,
        VALUE_OBJECT,
    };

    struct Value
    {
        Value();
        Value(int64_t integer);
        Value(double number);
        Value(bool boolean);
        Value(struct Object *object);
        ~Value();

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
            struct Object *object;
        };
    };

    bool operator==(const Value &left, const Value &right);
    bool operator!=(const Value &left, const Value &right);

    struct ValueHash
    {
        size_t operator()(const Value &v) const;
    };

    typedef std::unordered_map<Value, Value, ValueHash> ValueUnorderedMap;
}