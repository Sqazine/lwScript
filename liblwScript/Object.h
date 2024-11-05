#pragma once
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <map>
#include "Chunk.h"
#include "Token.h"
#include "Value.h"
namespace lwscript
{
#define IS_STR_OBJ(obj) ((obj)->type == OBJECT_STR)
#define IS_ARRAY_OBJ(obj) ((obj)->type == OBJECT_ARRAY)
#define IS_TABLE_OBJ(obj) ((obj)->type == OBJECT_DICT)
#define IS_ANONYMOUS_OBJ(obj) ((obj)->type == OBJECT_ANONYMOUS)
#define IS_FUNCTION_OBJ(obj) ((obj)->type == OBJECT_FUNCTION)
#define IS_UPVALUE_OBJ(obj) ((obj)->type == OBJECT_UPVALUE)
#define IS_CLOSURE_OBJ(obj) ((obj)->type == OBJECT_CLOSURE)
#define IS_NATIVE_FUNCTION_OBJ(obj) ((obj)->type == OBJECT_NATIVE_FUNCTION)
#define IS_REF_OBJ(obj) ((obj)->type == OBJECT_REF)
#define IS_CLASS_OBJ(obj) ((obj)->type == OBJECT_CLASS)
#define IS_CLASS_CLOSURE_BIND_OBJ(obj) ((obj)->type == OBJECT_CLASS_CLOSURE_BIND)
#define IS_ENUM_OBJ(obj) ((obj)->type == OBJECT_ENUM)
#define IS_MODULE_OBJ(obj) ((obj)->type == OBJECT_MODULE)

#define TO_STR_OBJ(obj) ((lwscript::StrObject *)(obj))
#define TO_ARRAY_OBJ(obj) ((lwscript::ArrayObject *)(obj))
#define TO_TABLE_OBJ(obj) ((lwscript::DictObject *)(obj))
#define TO_ANONYMOUS_OBJ(obj) ((lwscript::AnonymousObject *)(obj))
#define TO_FUNCTION_OBJ(obj) ((lwscript::FunctionObject *)(obj))
#define TO_UPVALUE_OBJ(obj) ((lwscript::UpValueObject *)(obj))
#define TO_CLOSURE_OBJ(obj) ((lwscript::ClosureObject *)(obj))
#define TO_NATIVE_FUNCTION_OBJ(obj) ((lwscript::NativeFunctionObject *)(obj))
#define TO_REF_OBJ(obj) ((lwscript::RefObject *)(obj))
#define TO_CLASS_OBJ(obj) ((lwscript::ClassObject *)(obj))
#define TO_CLASS_CLOSURE_BIND_OBJ(obj) ((lwscript::ClassClosureBindObject *)(obj))
#define TO_ENUM_OBJ(obj) ((lwscript::EnumObject *)(obj))
#define TO_MODULE_OBJ(obj) ((lwscript::ModuleObject *)(obj))

#define IS_NULL_VALUE(v) ((v).type == VALUE_NULL)
#define IS_INT_VALUE(v) ((v).type == VALUE_INT)
#define IS_REAL_VALUE(v) ((v).type == VALUE_REAL)
#define IS_BOOL_VALUE(v) ((v).type == VALUE_BOOL)
#define IS_OBJECT_VALUE(v) ((v).type == VALUE_OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STR_OBJ((v).object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ARRAY_OBJ((v).object))
#define IS_DICT_VALUE(v) (IS_OBJECT_VALUE(v) && IS_TABLE_OBJ((v).object))
#define IS_ANONYMOUS_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ANONYMOUS_OBJ((v).object))
#define IS_FUNCTION_VALUE(v) (IS_OBJECT_VALUE(v) && IS_FUNCTION_OBJ((v).object))
#define IS_UPVALUE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_UPVALUE_OBJ((v).object))
#define IS_CLOSURE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_CLOSURE_OBJ((v).object))
#define IS_NATIVE_FUNCTION_VALUE(v) (IS_OBJECT_VALUE(v) && IS_NATIVE_FUNCTION_OBJ((v).object))
#define IS_REF_VALUE(v) (IS_OBJECT_VALUE(v) && IS_REF_OBJ((v).object))
#define IS_CLASS_VALUE(v) (IS_OBJECT_VALUE(v) && IS_CLASS_OBJ((v).object))
#define IS_CLASS_CLOSURE_BIND_VALUE(v) (IS_OBJECT_VALUE(v) && IS_CLASS_CLOSURE_BIND_OBJ((v).object))
#define IS_ENUM_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ENUM_OBJ((v).object))
#define IS_MODULE_VALUE(v) (IS_OBJECT_VALUE(v) && IS_MODULE_OBJ((v).object))

#define TO_INT_VALUE(v) ((v).integer)
#define TO_REAL_VALUE(v) ((v).realnum)
#define TO_BOOL_VALUE(v) ((v).boolean)
#define TO_OBJECT_VALUE(v) ((v).object)
#define TO_STR_VALUE(v) (TO_STR_OBJ((v).object))
#define TO_ARRAY_VALUE(v) (TO_ARRAY_OBJ((v).object))
#define TO_DICT_VALUE(v) (TO_TABLE_OBJ((v).object))
#define TO_ANONYMOUS_VALUE(v) (TO_ANONYMOUS_OBJ((v).object))
#define TO_FUNCTION_VALUE(v) (TO_FUNCTION_OBJ((v).object))
#define TO_UPVALUE_VALUE(v) (TO_UPVALUE_OBJ((v).object))
#define TO_CLOSURE_VALUE(v) (TO_CLOSURE_OBJ((v).object))
#define TO_NATIVE_FUNCTION_VALUE(v) (TO_NATIVE_FUNCTION_OBJ((v).object))
#define TO_REF_VALUE(v) (TO_REF_OBJ((v).object))
#define TO_CLASS_VALUE(v) (TO_CLASS_OBJ((v).object))
#define TO_CLASS_CLOSURE_BIND_VALUE(v) (TO_CLASS_CLOSURE_BIND_OBJ((v).object))
#define TO_ENUM_VALUE(v) (TO_ENUM_OBJ((v).object))
#define TO_MODULE_VALUE(v) (TO_MODULE_OBJ((v).object))

    enum LWSCRIPT_API ObjectType
    {
        OBJECT_STR,
        OBJECT_ARRAY,
        OBJECT_DICT,
        OBJECT_ANONYMOUS,
        OBJECT_FUNCTION,
        OBJECT_UPVALUE,
        OBJECT_CLOSURE,
        OBJECT_NATIVE_FUNCTION,
        OBJECT_REF,
        OBJECT_CLASS,
        OBJECT_CLASS_CLOSURE_BIND,
        OBJECT_ENUM,
        OBJECT_MODULE
    };

    struct LWSCRIPT_API Object
    {
        Object(ObjectType type);
        virtual ~Object();

        virtual std::wstring ToString() const = 0;
        void Mark(class Allocator *allocator);
        void UnMark();
        virtual void Blacken(class Allocator *allocator);
        virtual bool IsEqualTo(Object *other) = 0;
        virtual Object *Clone() const = 0;

        const ObjectType type;
        bool marked{false};
        Object *next{nullptr};
    };

    struct LWSCRIPT_API StrObject : public Object
    {
        StrObject(std::wstring_view value);
        ~StrObject() override;

        std::wstring ToString() const override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        uint64_t NormalizeIdx(int64_t idx);

        std::wstring value{};
    };

    struct LWSCRIPT_API ArrayObject : public Object
    {
        ArrayObject();
        ArrayObject(const std::vector<struct Value> &elements);
        ~ArrayObject() override;

        std::wstring ToString() const override;
        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        uint64_t NormalizeIdx(int64_t idx);

        std::vector<struct Value> elements{};
    };

    struct LWSCRIPT_API DictObject : public Object
    {
        DictObject();
        DictObject(const ValueUnorderedMap &elements);
        ~DictObject() override;

        std::wstring ToString() const override;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        ValueUnorderedMap elements{};
    };

    struct LWSCRIPT_API AnonymousObject : public Object
    {
        AnonymousObject();
        AnonymousObject(const std::unordered_map<std::wstring, Value> &elements);
        ~AnonymousObject()override;

        std::wstring ToString() const override;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        std::unordered_map<std::wstring, Value> elements{};
    };

    struct LWSCRIPT_API FunctionObject : public Object
    {
        FunctionObject();
        FunctionObject(std::wstring_view name);
        ~FunctionObject() override;

        std::wstring ToString() const override;
        std::wstring ToStringWithChunk() const;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        // Function Cache Functions 
        void SetCache(const std::vector<Value>& arguments, const std::vector<Value>& result);
		bool GetCache(const std::vector<Value>& arguments, std::vector<Value>& result) const;
        void PrintCache();
        // Function Cache Functions

        uint8_t arity{0};
        VarArg varArg{VarArg::NONE};
        int8_t upValueCount{0};
        Chunk chunk{};
        std::wstring name{};

        ValueVecUnorderedMap caches;
    };

    struct LWSCRIPT_API UpValueObject : public Object
    {
        UpValueObject();
        UpValueObject(Value *location);
        ~UpValueObject() override;

        std::wstring ToString() const override;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        Value *location{nullptr};
        Value closed{};
        UpValueObject *nextUpValue{nullptr};
    };

    struct LWSCRIPT_API ClosureObject : public Object
    {
        ClosureObject();
        ClosureObject(FunctionObject *function);
        ~ClosureObject() override;

        std::wstring ToString() const override;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        FunctionObject *function{nullptr};
        std::vector<UpValueObject *> upvalues{};
    };

    using NativeFunction = std::function<bool(Value*,uint32_t, const Token *, Value&)>;

    struct LWSCRIPT_API NativeFunctionObject : public Object
    {
        NativeFunctionObject();
        NativeFunctionObject(NativeFunction f);
        ~NativeFunctionObject() override;

        std::wstring ToString() const override;

        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        NativeFunction fn{};
    };

    struct LWSCRIPT_API RefObject : public Object
    {
        RefObject(Value *pointer);
        ~RefObject() override;

        std::wstring ToString() const override;

        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        Value *pointer{};
    };

    struct LWSCRIPT_API ClassObject : public Object
    {
        ClassObject();
        ClassObject(std::wstring_view name);
        ~ClassObject() override;

        std::wstring ToString() const override;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        bool GetMember(const std::wstring &name, Value &retV);
        bool GetParentMember(const std::wstring &name, Value &retV);

        std::wstring name{};
        std::map<int32_t, ClosureObject *> constructors{}; // argument count as key for now
        std::unordered_map<std::wstring, Value> members{};
        std::map<std::wstring, ClassObject *> parents{};
    };

    struct LWSCRIPT_API ClassClosureBindObject : public Object
    {
        ClassClosureBindObject();
        ClassClosureBindObject(const Value &receiver, ClosureObject *cl);
        ~ClassClosureBindObject() override;

        std::wstring ToString() const override;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        Value receiver;
        ClosureObject *closure;
    };

    struct LWSCRIPT_API EnumObject : public Object
    {
        EnumObject();
        EnumObject(const std::wstring &name, const std::unordered_map<std::wstring, Value> &pairs);
        ~EnumObject() override;

        std::wstring ToString() const override;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        bool GetMember(const std::wstring &name, Value &retV);

        std::wstring name;
        std::unordered_map<std::wstring, Value> pairs;
    };

    struct LWSCRIPT_API ModuleObject : public Object
    {
        ModuleObject();
        ModuleObject(const std::wstring &name, const std::unordered_map<std::wstring, Value> &values);
        ~ModuleObject() override;

        std::wstring ToString() const override;

        void Blacken(class Allocator *allocator) override;
        bool IsEqualTo(Object *other) override;
        Object *Clone() const override;

        bool GetMember(const std::wstring &name, Value &retV);

        std::wstring name;
        std::unordered_map<std::wstring, Value> values;
    };
}