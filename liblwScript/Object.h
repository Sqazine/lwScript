#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Chunk.h"
namespace lws
{
#define IS_STR_OBJ(obj) (obj->Type() == OBJECT_STR)
#define IS_ARRAY_OBJ(obj) (obj->Type() == OBJECT_ARRAY)
#define IS_TABLE_OBJ(obj) (obj->Type() == OBJECT_TABLE)
#define IS_FUNCTION_OBJ(obj) (obj->Type() == OBJECT_FUNCTION)

#define TO_STR_OBJ(obj) ((lws::StrObject *)obj)
#define TO_ARRAY_OBJ(obj) ((lws::ArrayObject *)obj)
#define TO_TABLE_OBJ(obj) ((lws::TableObject *)obj)
#define TO_FUNCTION_OBJ(obj) ((lws::FunctionObject *)obj)

#define IS_NULL_VALUE(v) (v.Type() == VALUE_NULL)
#define IS_INT_VALUE(v) (v.Type() == VALUE_INT)
#define IS_REAL_VALUE(v) (v.Type() == VALUE_REAL)
#define IS_BOOL_VALUE(v) (v.Type() == VALUE_BOOL)
#define IS_OBJECT_VALUE(v) (v.Type() == VALUE_OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STR_OBJ(v.object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ARRAY_OBJ(v.object))
#define IS_TABLE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_TABLE_OBJ(v.object))
#define IS_FUNCTION_VALUE(v) (IS_OBJECT_VALUE(v) && IS_FUNCTION_OBJ(v.object))
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
#define TO_FUNCTION_VALUE(v) (TO_FUNCTION_OBJ(v.object))
#define TO_LAMBDA_VALUE(v) (TO_LAMBDA_OBJ(v.object))
#define TO_CLASS_VALUE(v) (TO_CLASS_OBJ(v.object))
#define TO_REF_VALUE(v) (TO_REF_OBJ(v.object))

    enum ObjectType
    {
        OBJECT_STR,
        OBJECT_ARRAY,
        OBJECT_TABLE,
        OBJECT_FUNCTION,
    };

    struct Object
    {
        Object() : marked(false), next(nullptr) {}
        virtual ~Object() {}

        virtual std::wstring Stringify() const = 0;
        virtual ObjectType Type() const = 0;
        virtual void Mark() = 0;
        virtual void UnMark() = 0;
        virtual bool IsEqualTo(Object *other) = 0;

        bool marked;
        Object *next;
    };

    struct StrObject : public Object
    {
        StrObject(std::wstring_view value);
        ~StrObject();

        std::wstring Stringify() const override;
        ObjectType Type() const override;
        void Mark() override;
        void UnMark() override;
        bool IsEqualTo(Object *other) override;

        std::wstring value;
    };

    struct ArrayObject : public Object
    {
        ArrayObject();
        ArrayObject(const std::vector<struct Value> &elements);
        ~ArrayObject();

        std::wstring Stringify() const override;
        ObjectType Type() const override;
        void Mark() override;
        void UnMark() override;

        bool IsEqualTo(Object *other) override;

        std::vector<struct Value> elements;
    };

    struct ValueHash
    {
        size_t operator()(const Value &v) const;
    };

    typedef std::unordered_map<Value, Value, ValueHash> ValueUnorderedMap;

    struct TableObject : public Object
    {
        TableObject();
        TableObject(const ValueUnorderedMap &elements);
        ~TableObject();

        std::wstring Stringify() const override;
        ObjectType Type() const override;
        void Mark() override;
        void UnMark() override;

        bool IsEqualTo(Object *other) override;

        ValueUnorderedMap elements;
    };

    struct FunctionObject : public Object
    {
        FunctionObject();
        FunctionObject(std::wstring_view name);
        ~FunctionObject();

        std::wstring Stringify() const override;
        ObjectType Type() const override;
        void Mark() override;
        void UnMark() override;

        bool IsEqualTo(Object *other) override;

        int32_t arity;
        int32_t upValueCount;
        Chunk chunk;
        std::wstring name;
    };
}