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
#define IS_STR_OBJ(obj) ((obj)->kind == ObjectKind::STR)
#define IS_ARRAY_OBJ(obj) ((obj)->kind == ObjectKind::ARRAY)
#define IS_TABLE_OBJ(obj) ((obj)->kind == ObjectKind::DICT)
#define IS_STRUCT_OBJ(obj) ((obj)->kind == ObjectKind::STRUCT)
#define IS_FUNCTION_OBJ(obj) ((obj)->kind == ObjectKind::FUNCTION)
#define IS_UPVALUE_OBJ(obj) ((obj)->kind == ObjectKind::UPVALUE)
#define IS_CLOSURE_OBJ(obj) ((obj)->kind == ObjectKind::CLOSURE)
#define IS_NATIVE_FUNCTION_OBJ(obj) ((obj)->kind == ObjectKind::NATIVE_FUNCTION)
#define IS_REF_OBJ(obj) ((obj)->kind == ObjectKind::REF)
#define IS_CLASS_OBJ(obj) ((obj)->kind == ObjectKind::CLASS)
#define IS_CLASS_CLOSURE_BIND_OBJ(obj) ((obj)->kind == ObjectKind::CLASS_CLOSURE_BIND)
#define IS_ENUM_OBJ(obj) ((obj)->kind == ObjectKind::ENUM)
#define IS_MODULE_OBJ(obj) ((obj)->kind == ObjectKind::MODULE)

#define TO_STR_OBJ(obj) ((lwscript::StrObject *)(obj))
#define TO_ARRAY_OBJ(obj) ((lwscript::ArrayObject *)(obj))
#define TO_TABLE_OBJ(obj) ((lwscript::DictObject *)(obj))
#define TO_STRUCT_OBJ(obj) ((lwscript::StructObject *)(obj))
#define TO_FUNCTION_OBJ(obj) ((lwscript::FunctionObject *)(obj))
#define TO_UPVALUE_OBJ(obj) ((lwscript::UpValueObject *)(obj))
#define TO_CLOSURE_OBJ(obj) ((lwscript::ClosureObject *)(obj))
#define TO_NATIVE_FUNCTION_OBJ(obj) ((lwscript::NativeFunctionObject *)(obj))
#define TO_REF_OBJ(obj) ((lwscript::RefObject *)(obj))
#define TO_CLASS_OBJ(obj) ((lwscript::ClassObject *)(obj))
#define TO_CLASS_CLOSURE_BIND_OBJ(obj) ((lwscript::ClassClosureBindObject *)(obj))
#define TO_ENUM_OBJ(obj) ((lwscript::EnumObject *)(obj))
#define TO_MODULE_OBJ(obj) ((lwscript::ModuleObject *)(obj))

#define IS_NULL_VALUE(v) ((v).kind == ValueKind::NIL)
#define IS_INT_VALUE(v) ((v).kind == ValueKind::INT)
#define IS_REAL_VALUE(v) ((v).kind == ValueKind::REAL)
#define IS_BOOL_VALUE(v) ((v).kind == ValueKind::BOOL)
#define IS_OBJECT_VALUE(v) ((v).kind == ValueKind::OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STR_OBJ((v).object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ARRAY_OBJ((v).object))
#define IS_DICT_VALUE(v) (IS_OBJECT_VALUE(v) && IS_TABLE_OBJ((v).object))
#define IS_STRUCT_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STRUCT_OBJ((v).object))
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
#define TO_STRUCT_VALUE(v) (TO_STRUCT_OBJ((v).object))
#define TO_FUNCTION_VALUE(v) (TO_FUNCTION_OBJ((v).object))
#define TO_UPVALUE_VALUE(v) (TO_UPVALUE_OBJ((v).object))
#define TO_CLOSURE_VALUE(v) (TO_CLOSURE_OBJ((v).object))
#define TO_NATIVE_FUNCTION_VALUE(v) (TO_NATIVE_FUNCTION_OBJ((v).object))
#define TO_REF_VALUE(v) (TO_REF_OBJ((v).object))
#define TO_CLASS_VALUE(v) (TO_CLASS_OBJ((v).object))
#define TO_CLASS_CLOSURE_BIND_VALUE(v) (TO_CLASS_CLOSURE_BIND_OBJ((v).object))
#define TO_ENUM_VALUE(v) (TO_ENUM_OBJ((v).object))
#define TO_MODULE_VALUE(v) (TO_MODULE_OBJ((v).object))

    enum LWSCRIPT_API ObjectKind : uint8_t
    {
        STR,
        ARRAY,
        DICT,
        STRUCT,
        FUNCTION,
        UPVALUE,
        CLOSURE,
        NATIVE_FUNCTION,
        REF,
        CLASS,
        CLASS_CLOSURE_BIND,
        ENUM,
        MODULE
    };

    struct LWSCRIPT_API Object
    {
        Object(ObjectKind kind);
        virtual ~Object();

        virtual STD_STRING ToString() const = 0;
        void Mark();
        void UnMark();
        virtual void Blacken();
        virtual bool IsEqualTo(Object *other) = 0;
        virtual std::vector<uint8_t> Serialize() const = 0;

        const ObjectKind kind;
        bool marked{false};
        Object *next{nullptr};
    };

    struct LWSCRIPT_API StrObject : public Object
    {
        StrObject(STD_STRING_VIEW value);
        ~StrObject() override;

        STD_STRING ToString() const override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        STD_STRING value{};
    };

    struct LWSCRIPT_API ArrayObject : public Object
    {
        ArrayObject();
        ArrayObject(const std::vector<struct Value> &elements);
        ~ArrayObject() override;

        STD_STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        std::vector<struct Value> elements{};
    };

    struct LWSCRIPT_API DictObject : public Object
    {
        DictObject();
        DictObject(const ValueUnorderedMap &elements);
        ~DictObject() override;

        STD_STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        ValueUnorderedMap elements{};
    };

    struct LWSCRIPT_API StructObject : public Object
    {
        StructObject();
        StructObject(const std::unordered_map<STD_STRING, Value> &elements);
        ~StructObject() override;

        STD_STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        std::unordered_map<STD_STRING, Value> elements{};
    };

    struct LWSCRIPT_API FunctionObject : public Object
    {
        FunctionObject();
        FunctionObject(STD_STRING_VIEW name);
        ~FunctionObject() override;

        STD_STRING ToString() const override;
#ifndef NDEBUG
        STD_STRING ToStringWithChunk() const;
#endif

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

#ifdef FUNCTION_CACHE_OPT
        void SetCache(size_t hash, const std::vector<Value> &result);
        bool GetCache(size_t hash, std::vector<Value> &result) const;
        void PrintCache();

        std::unordered_map<size_t, std::vector<Value>> caches;
#endif

        uint8_t arity{0};
        VarArg varArg{VarArg::NONE};
        int8_t upValueCount{0};
        Chunk chunk{};
        STD_STRING name{};
    };

    struct LWSCRIPT_API UpValueObject : public Object
    {
        UpValueObject();
        UpValueObject(Value *location);
        ~UpValueObject() override;

        STD_STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value *location{nullptr};
        Value closed{};
        UpValueObject *nextUpValue{nullptr};
    };

    struct LWSCRIPT_API ClosureObject : public Object
    {
        ClosureObject();
        ClosureObject(FunctionObject *function);
        ~ClosureObject() override;

        STD_STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        FunctionObject *function{nullptr};
        std::vector<UpValueObject *> upvalues{};
    };

    using NativeFunction = std::function<bool(Value *, uint32_t, const Token *, Value &)>;

    struct LWSCRIPT_API NativeFunctionObject : public Object
    {
        NativeFunctionObject();
        NativeFunctionObject(NativeFunction f);
        ~NativeFunctionObject() override;

        STD_STRING ToString() const override;

        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        NativeFunction fn{};
    };

    struct LWSCRIPT_API RefObject : public Object
    {
        RefObject(Value *pointer);
        ~RefObject() override;

        STD_STRING ToString() const override;

        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value *pointer{nullptr};
    };

    struct LWSCRIPT_API ClassObject : public Object
    {
        ClassObject();
        ClassObject(STD_STRING_VIEW name);
        ~ClassObject() override;

        STD_STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STD_STRING &name, Value &retV);
        bool GetParentMember(const STD_STRING &name, Value &retV);

        STD_STRING name{};
        std::map<int32_t, ClosureObject *> constructors{}; // argument count as key for now
        std::unordered_map<STD_STRING, Value> members{};
        std::map<STD_STRING, ClassObject *> parents{};
    };

    struct LWSCRIPT_API ClassClosureBindObject : public Object
    {
        ClassClosureBindObject();
        ClassClosureBindObject(const Value &receiver, ClosureObject *cl);
        ~ClassClosureBindObject() override;

        STD_STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value receiver{};
        ClosureObject *closure{nullptr};
    };

    struct LWSCRIPT_API EnumObject : public Object
    {
        EnumObject();
        EnumObject(const STD_STRING &name, const std::unordered_map<STD_STRING, Value> &pairs);
        ~EnumObject() override;

        STD_STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STD_STRING &name, Value &retV);

        STD_STRING name{};
        std::unordered_map<STD_STRING, Value> pairs{};
    };

    struct LWSCRIPT_API ModuleObject : public Object
    {
        ModuleObject();
        ModuleObject(const STD_STRING &name, const std::unordered_map<STD_STRING, Value> &values);
        ~ModuleObject() override;

        STD_STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STD_STRING &name, Value &retV);

        STD_STRING name{};
        std::unordered_map<STD_STRING, Value> values{};
    };
}