#pragma once
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <map>
#include "Chunk.h"
namespace lws
{
#define IS_STR_OBJ(obj) (obj->type == OBJECT_STR)
#define IS_ARRAY_OBJ(obj) (obj->type == OBJECT_ARRAY)
#define IS_TABLE_OBJ(obj) (obj->type == OBJECT_TABLE)
#define IS_FUNCTION_OBJ(obj) (obj->type == OBJECT_FUNCTION)
#define IS_UPVALUE_OBJ(obj) (obj->type == OBJECT_UPVALUE)
#define IS_CLOSURE_OBJ(obj) (obj->type == OBJECT_CLOSURE)
#define IS_NATIVE_FUNCTION_OBJ(obj) (obj->type == OBJECT_NATIVE_FUNCTION)
#define IS_REF_OBJ(obj) (obj->type == OBJECT_REF)
#define IS_CLASS_OBJ(obj) (obj->type == OBJECT_CLASS)
#define IS_CLASS_CLOSURE_BIND_OBJ(obj) (obj->type == OBJECT_CLASS_CLOSURE_BIND)
#define IS_ENUM_OBJ(obj) (obj->type == OBJECT_ENUM)

#define TO_STR_OBJ(obj) ((lws::StrObject *)obj)
#define TO_ARRAY_OBJ(obj) ((lws::ArrayObject *)obj)
#define TO_TABLE_OBJ(obj) ((lws::TableObject *)obj)
#define TO_FUNCTION_OBJ(obj) ((lws::FunctionObject *)obj)
#define TO_UPVALUE_OBJ(obj) ((lws::UpValueObject *)obj)
#define TO_CLOSURE_OBJ(obj) ((lws::ClosureObject *)obj)
#define TO_NATIVE_FUNCTION_OBJ(obj) ((lws::NativeFunctionObject *)obj)
#define TO_REF_OBJ(obj) ((lws::RefObject *)obj)
#define TO_CLASS_OBJ(obj) ((lws::ClassObject *)obj)
#define TO_CLASS_CLOSURE_BIND_OBJ(obj) ((lws::ClassClosureBindObject *)obj)
#define TO_ENUM_OBJ(obj) ((lws::EnumObject *)obj)

#define IS_NULL_VALUE(v) (v.type == VALUE_NULL)
#define IS_INT_VALUE(v) (v.type == VALUE_INT)
#define IS_REAL_VALUE(v) (v.type == VALUE_REAL)
#define IS_BOOL_VALUE(v) (v.type == VALUE_BOOL)
#define IS_OBJECT_VALUE(v) (v.type == VALUE_OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STR_OBJ(v.object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ARRAY_OBJ(v.object))
#define IS_TABLE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_TABLE_OBJ(v.object))
#define IS_FUNCTION_VALUE(v) (IS_OBJECT_VALUE(v) && IS_FUNCTION_OBJ(v.object))
#define IS_UPVALUE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_UPVALUE_OBJ(v.object))
#define IS_CLOSURE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_CLOSURE_OBJ(v.object))
#define IS_NATIVE_FUNCTION_VALUE(v) (IS_OBJECT_VALUE(v) && IS_NATIVE_FUNCTION_OBJ(v.object))
#define IS_REF_VALUE(v) (IS_OBJECT_VALUE(v) && IS_REF_OBJ(v.object))
#define IS_CLASS_VALUE(v) (IS_OBJECT_VALUE(v) && IS_CLASS_OBJ(v.object))
#define IS_CLASS_CLOSURE_BIND_VALUE(v) (IS_OBJECT_VALUE(v) && IS_CLASS_CLOSURE_BIND_OBJ(v.object))
#define IS_ENUM_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ENUM_OBJ(v.object))

#define TO_INT_VALUE(v) (v.integer)
#define TO_REAL_VALUE(v) (v.realnum)
#define TO_BOOL_VALUE(v) (v.boolean)
#define TO_OBJECT_VALUE(v) (v.object)
#define TO_STR_VALUE(v) (TO_STR_OBJ(v.object)->value)
#define TO_ARRAY_VALUE(v) (TO_ARRAY_OBJ(v.object))
#define TO_TABLE_VALUE(v) (TO_TABLE_OBJ(v.object))
#define TO_FUNCTION_VALUE(v) (TO_FUNCTION_OBJ(v.object))
#define TO_UPVALUE_VALUE(v) (TO_UPVALUE_OBJ(v.object))
#define TO_CLOSURE_VALUE(v) (TO_CLOSURE_OBJ(v.object))
#define TO_NATIVE_FUNCTION_VALUE(v) (TO_NATIVE_FUNCTION_OBJ(v.object))
#define TO_REF_VALUE(v) (TO_REF_OBJ(v.object))
#define TO_CLASS_VALUE(v) (TO_CLASS_OBJ(v.object))
#define TO_CLASS_CLOSURE_BIND_VALUE(v) (TO_CLASS_CLOSURE_BIND_OBJ(v.object))
#define TO_ENUM_VALUE(v) (TO_ENUM_OBJ(v.object))

    enum ObjectType
    {
        OBJECT_STR,
        OBJECT_ARRAY,
        OBJECT_TABLE,
        OBJECT_FUNCTION,
        OBJECT_UPVALUE,
        OBJECT_CLOSURE,
        OBJECT_NATIVE_FUNCTION,
        OBJECT_REF,
        OBJECT_CLASS,
        OBJECT_CLASS_CLOSURE_BIND,
        OBJECT_ENUM
    };

    struct Object
    {
        Object(ObjectType type);
        virtual ~Object();

        virtual std::wstring Stringify(bool outputOpCodeIfExists = false) const = 0;
        void Mark(class VM *vm);
        void UnMark();
        virtual void Blacken(class VM *vm);
        virtual bool IsEqualTo(Object *other) = 0;
        virtual Object *Clone() const = 0;

        const ObjectType type;
        bool marked;
        Object *next;
    };

    struct StrObject : public Object
    {
        StrObject(std::wstring_view value);
        ~StrObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        std::wstring value;
    };

    struct ArrayObject : public Object
    {
        ArrayObject();
        ArrayObject(const std::vector<struct Value> &elements);
        ~ArrayObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
        void Blacken(class VM *vm) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

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

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        void Blacken(class VM *vm) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        ValueUnorderedMap elements;
    };

    struct FunctionObject : public Object
    {
        FunctionObject();
        FunctionObject(std::wstring_view name);
        ~FunctionObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        void Blacken(class VM *vm) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        int32_t arity;
        int32_t upValueCount;
        Chunk chunk;
        std::wstring name;
    };

    struct UpValueObject : public Object
    {
        UpValueObject();
        UpValueObject(Value *location);
        ~UpValueObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        void Blacken(class VM *vm) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        Value *location;
        Value closed;
        UpValueObject *nextUpValue;
    };

    struct ClosureObject : public Object
    {
        ClosureObject();
        ClosureObject(FunctionObject *function);
        ~ClosureObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        void Blacken(class VM *vm) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        FunctionObject *function;
        std::vector<UpValueObject *> upvalues;
    };

    using NativeFunction = std::function<Value(const std::vector<Value> &)>;

    struct NativeFunctionObject : public Object
    {
        NativeFunctionObject();
        NativeFunctionObject(NativeFunction f);
        ~NativeFunctionObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        NativeFunction fn;
    };

    struct RefObject : public Object
    {
        RefObject(Value *pointer);
        ~RefObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        Value *pointer;
    };

    struct ClassObject : public Object
    {
        ClassObject();
        ClassObject(std::wstring_view name);
        ~ClassObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        void Blacken(class VM *vm) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        bool GetMember(const std::wstring &name, Value &retV);
        bool GetParentMember(const std::wstring &name, Value &retV);

        std::wstring name;
        std::map<int32_t, ClosureObject *> constructors; //argument count as key
        std::unordered_map<std::wstring, Value> members;
        std::map<std::wstring, ClassObject *> parents;
    };

    struct ClassClosureBindObject : public Object
    {
        ClassClosureBindObject();
        ClassClosureBindObject(const Value &receiver, ClosureObject *cl);
        ~ClassClosureBindObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        void Blacken(class VM *vm) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        Value receiver;
        ClosureObject *closure;
    };

    struct EnumObject : public Object
    {
        EnumObject();
        EnumObject(const std::wstring &name, const std::unordered_map<std::wstring, Value> &pairs);
        ~EnumObject();

        std::wstring Stringify(bool outputOpCodeIfExists = false) const override;
       
        void Blacken(class VM *vm) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        bool GetMember(const std::wstring &name, Value &retV);

        std::wstring name;
        std::unordered_map<std::wstring, Value> pairs;
    };

}