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
#define LWS_IS_STR_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::STR)
#define LWS_IS_ARRAY_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::ARRAY)
#define LWS_IS_TABLE_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::DICT)
#define LWS_IS_STRUCT_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::STRUCT)
#define LWS_IS_FUNCTION_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::FUNCTION)
#define LWS_IS_UPVALUE_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::UPVALUE)
#define LWS_IS_CLOSURE_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::CLOSURE)
#define LWS_IS_NATIVE_FUNCTION_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::NATIVE_FUNCTION)
#define LWS_IS_REF_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::REF)
#define LWS_IS_CLASS_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::CLASS)
#define LWS_IS_CLASS_CLOSURE_BIND_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::CLASS_CLOSURE_BIND)
#define LWS_IS_ENUM_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::ENUM)
#define LWS_IS_MODULE_OBJ(obj) ((obj)->kind == ::lwscript::ObjectKind::MODULE)

#define LWS_TO_STR_OBJ(obj) ((::lwscript::StrObject *)(obj))
#define LWS_TO_ARRAY_OBJ(obj) ((::lwscript::ArrayObject *)(obj))
#define LWS_TO_TABLE_OBJ(obj) ((::lwscript::DictObject *)(obj))
#define LWS_TO_STRUCT_OBJ(obj) ((::lwscript::StructObject *)(obj))
#define LWS_TO_FUNCTION_OBJ(obj) ((::lwscript::FunctionObject *)(obj))
#define LWS_TO_UPVALUE_OBJ(obj) ((::lwscript::UpValueObject *)(obj))
#define LWS_TO_CLOSURE_OBJ(obj) ((::lwscript::ClosureObject *)(obj))
#define LWS_TO_NATIVE_FUNCTION_OBJ(obj) ((::lwscript::NativeFunctionObject *)(obj))
#define LWS_TO_REF_OBJ(obj) ((::lwscript::RefObject *)(obj))
#define LWS_TO_CLASS_OBJ(obj) ((::lwscript::ClassObject *)(obj))
#define LWS_TO_CLASS_CLOSURE_BIND_OBJ(obj) ((::lwscript::ClassClosureBindObject *)(obj))
#define LWS_TO_ENUM_OBJ(obj) ((::lwscript::EnumObject *)(obj))
#define LWS_TO_MODULE_OBJ(obj) ((::lwscript::ModuleObject *)(obj))

#define LWS_IS_NULL_VALUE(v) ((v).kind == ::lwscript::ValueKind::NIL)
#define LWS_IS_INT_VALUE(v) ((v).kind == ::lwscript::ValueKind::INT)
#define LWS_IS_REAL_VALUE(v) ((v).kind == ::lwscript::ValueKind::REAL)
#define LWS_IS_BOOL_VALUE(v) ((v).kind == ::lwscript::ValueKind::BOOL)
#define LWS_IS_OBJECT_VALUE(v) ((v).kind == ::lwscript::ValueKind::OBJECT)
#define LWS_IS_STR_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_STR_OBJ((v).object))
#define LWS_IS_ARRAY_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_ARRAY_OBJ((v).object))
#define LWS_IS_DICT_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_TABLE_OBJ((v).object))
#define LWS_IS_STRUCT_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_STRUCT_OBJ((v).object))
#define LWS_IS_FUNCTION_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_FUNCTION_OBJ((v).object))
#define LWS_IS_UPVALUE_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_UPVALUE_OBJ((v).object))
#define LWS_IS_CLOSURE_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_CLOSURE_OBJ((v).object))
#define LWS_IS_NATIVE_FUNCTION_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_NATIVE_FUNCTION_OBJ((v).object))
#define LWS_IS_REF_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_REF_OBJ((v).object))
#define LWS_IS_CLASS_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_CLASS_OBJ((v).object))
#define LWS_IS_CLASS_CLOSURE_BIND_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_CLASS_CLOSURE_BIND_OBJ((v).object))
#define LWS_IS_ENUM_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_ENUM_OBJ((v).object))
#define LWS_IS_MODULE_VALUE(v) (LWS_IS_OBJECT_VALUE(v) && LWS_IS_MODULE_OBJ((v).object))

#define LWS_TO_INT_VALUE(v) ((v).integer)
#define LWS_TO_REAL_VALUE(v) ((v).realnum)
#define LWS_TO_BOOL_VALUE(v) ((v).boolean)
#define LWS_TO_OBJECT_VALUE(v) ((v).object)
#define LWS_TO_STR_VALUE(v) (LWS_TO_STR_OBJ((v).object))
#define LWS_TO_ARRAY_VALUE(v) (LWS_TO_ARRAY_OBJ((v).object))
#define LWS_TO_DICT_VALUE(v) (LWS_TO_TABLE_OBJ((v).object))
#define LWS_TO_STRUCT_VALUE(v) (LWS_TO_STRUCT_OBJ((v).object))
#define LWS_TO_FUNCTION_VALUE(v) (LWS_TO_FUNCTION_OBJ((v).object))
#define LWS_TO_UPVALUE_VALUE(v) (LWS_TO_UPVALUE_OBJ((v).object))
#define LWS_TO_CLOSURE_VALUE(v) (LWS_TO_CLOSURE_OBJ((v).object))
#define LWS_TO_NATIVE_FUNCTION_VALUE(v) (LWS_TO_NATIVE_FUNCTION_OBJ((v).object))
#define LWS_TO_REF_VALUE(v) (LWS_TO_REF_OBJ((v).object))
#define LWS_TO_CLASS_VALUE(v) (LWS_TO_CLASS_OBJ((v).object))
#define LWS_TO_CLASS_CLOSURE_BIND_VALUE(v) (LWS_TO_CLASS_CLOSURE_BIND_OBJ((v).object))
#define LWS_TO_ENUM_VALUE(v) (LWS_TO_ENUM_OBJ((v).object))
#define LWS_TO_MODULE_VALUE(v) (LWS_TO_MODULE_OBJ((v).object))

    enum LWS_API ObjectKind : uint8_t
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

    struct LWS_API Object
    {
        Object(ObjectKind kind);
        virtual ~Object();

        virtual STRING ToString() const = 0;
        void Mark();
        void UnMark();
        virtual void Blacken();
        virtual bool IsEqualTo(Object *other) = 0;
        virtual std::vector<uint8_t> Serialize() const = 0;

        const ObjectKind kind;
        bool marked{false};
        Object *next{nullptr};
    };

    struct LWS_API StrObject : public Object
    {
        StrObject(STRING_VIEW value);
        ~StrObject() override;

        STRING ToString() const override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        STRING value{};
    };

    struct LWS_API ArrayObject : public Object
    {
        ArrayObject();
        ArrayObject(const std::vector<struct Value> &elements);
        ~ArrayObject() override;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        std::vector<struct Value> elements{};
    };

    struct LWS_API DictObject : public Object
    {
        DictObject();
        DictObject(const ValueUnorderedMap &elements);
        ~DictObject() override;

        STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        ValueUnorderedMap elements{};
    };

    struct LWS_API StructObject : public Object
    {
        StructObject();
        StructObject(const std::unordered_map<STRING, Value> &elements);
        ~StructObject() override;

        STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        std::unordered_map<STRING, Value> elements{};
    };

    struct LWS_API FunctionObject : public Object
    {
        FunctionObject();
        FunctionObject(STRING_VIEW name);
        ~FunctionObject() override;

        STRING ToString() const override;
#ifndef NDEBUG
        STRING ToStringWithChunk() const;
#endif

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

#ifdef LWS_FUNCTION_CACHE_OPT
        void SetCache(size_t hash, const std::vector<Value> &result);
        bool GetCache(size_t hash, std::vector<Value> &result) const;
        void PrintCache();

        std::unordered_map<size_t, std::vector<Value>> caches;
#endif

        uint8_t arity{0};
        VarArg varArg{VarArg::NONE};
        int8_t upValueCount{0};
        Chunk chunk{};
        STRING name{};
    };

    struct LWS_API UpValueObject : public Object
    {
        UpValueObject();
        UpValueObject(Value *location);
        ~UpValueObject() override;

        STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value *location{nullptr};
        Value closed{};
        UpValueObject *nextUpValue{nullptr};
    };

    struct LWS_API ClosureObject : public Object
    {
        ClosureObject();
        ClosureObject(FunctionObject *function);
        ~ClosureObject() override;

        STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        FunctionObject *function{nullptr};
        std::vector<UpValueObject *> upvalues{};
    };

    using NativeFunction = std::function<bool(Value *, uint32_t, const Token *, Value &)>;

    struct LWS_API NativeFunctionObject : public Object
    {
        NativeFunctionObject();
        NativeFunctionObject(NativeFunction f);
        ~NativeFunctionObject() override;

        STRING ToString() const override;

        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        NativeFunction fn{};
    };

    struct LWS_API RefObject : public Object
    {
        RefObject(Value *pointer);
        ~RefObject() override;

        STRING ToString() const override;

        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value *pointer{nullptr};
    };

    struct LWS_API ClassObject : public Object
    {
        ClassObject();
        ClassObject(STRING_VIEW name);
        ~ClassObject() override;

        STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STRING &name, Value &retV);
        bool GetParentMember(const STRING &name, Value &retV);

        STRING name{};
        std::map<int32_t, ClosureObject *> constructors{}; // argument count as key for now
        std::unordered_map<STRING, Value> members{};
        std::map<STRING, ClassObject *> parents{};
    };

    struct LWS_API ClassClosureBindObject : public Object
    {
        ClassClosureBindObject();
        ClassClosureBindObject(const Value &receiver, ClosureObject *cl);
        ~ClassClosureBindObject() override;

        STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value receiver{};
        ClosureObject *closure{nullptr};
    };

    struct LWS_API EnumObject : public Object
    {
        EnumObject();
        EnumObject(const STRING &name, const std::unordered_map<STRING, Value> &pairs);
        ~EnumObject() override;

        STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STRING &name, Value &retV);

        STRING name{};
        std::unordered_map<STRING, Value> pairs{};
    };

    struct LWS_API ModuleObject : public Object
    {
        ModuleObject();
        ModuleObject(const STRING &name, const std::unordered_map<STRING, Value> &values);
        ~ModuleObject() override;

        STRING ToString() const override;

        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STRING &name, Value &retV);

        STRING name{};
        std::unordered_map<STRING, Value> values{};
    };
}