#include "Object.h"
#include "Value.h"
#include "Chunk.h"
#include "Utils.h"
#include "VM.h"
namespace lws
{

    Object::Object()
        : marked(false), next(nullptr)
    {
    }
    Object::~Object()
    {
    }

    void Object::Mark(VM *vm)
    {
        if (marked)
            return;
#ifdef GC_DEBUG
        std::wcout << L"0x" << (void *)this << L" mark: " << Stringify() << std::endl;
#endif
        marked = true;
        vm->mGrayObjects.emplace_back(this);
    }
    void Object ::UnMark()
    {
        if (!marked)
            return;
#ifdef GC_DEBUG
        std::wcout << L"0x" << (void *)this << L" unMark: " << Stringify() << std::endl;
#endif
        marked = false;
    }

    void Object::Blacken(VM *vm)
    {
#ifdef GC_DEBUG
        std::wcout << L"0x" << (void *)this << L" blacken: " << Stringify() << std::endl;
#endif
    }

    StrObject::StrObject(std::wstring_view value)
        : value(value)
    {
    }
    StrObject::~StrObject()
    {
    }
    std::wstring StrObject::Stringify(bool outputOpCodeIfExists) const
    {
        return value;
    }
    ObjectType StrObject::Type() const
    {
        return OBJECT_STR;
    }
    bool StrObject::IsEqualTo(Object *other)
    {
        if (!IS_STR_OBJ(other))
            return false;
        return value == TO_STR_OBJ(other)->value;
    }

    Object *StrObject::Clone() const
    {
        return new StrObject(value);
    }

    ArrayObject::ArrayObject()
    {
    }
    ArrayObject::ArrayObject(const std::vector<Value> &elements)
        : elements(elements)
    {
    }
    ArrayObject::~ArrayObject()
    {
    }

    std::wstring ArrayObject::Stringify(bool outputOpCodeIfExists) const
    {
        std::wstring result = L"[";
        if (!elements.empty())
        {
            for (const auto &e : elements)
                result += e.Stringify(outputOpCodeIfExists) + L",";
            result = result.substr(0, result.size() - 1);
        }
        result += L"]";
        return result;
    }
    ObjectType ArrayObject::Type() const
    {
        return OBJECT_ARRAY;
    }

    void ArrayObject::Blacken(VM *vm)
    {
        Object::Blacken(vm);
        for (auto &e : elements)
            e.Mark(vm);
    }

    bool ArrayObject::IsEqualTo(Object *other)
    {
        if (!IS_ARRAY_OBJ(other))
            return false;

        ArrayObject *arrayOther = TO_ARRAY_OBJ(other);

        if (arrayOther->elements.size() != elements.size())
            return false;

        for (size_t i = 0; i < elements.size(); ++i)
            if (elements[i] != arrayOther->elements[i])
                return false;

        return true;
    }

    Object *ArrayObject::Clone() const
    {
        std::vector<Value> eles(this->elements.size());
        for (int32_t i = 0; i < eles.size(); ++i)
            eles[i] = this->elements[i].Clone();
        return new ArrayObject(eles);
    }

    TableObject::TableObject()
    {
    }
    TableObject::TableObject(const ValueUnorderedMap &elements)
        : elements(elements)
    {
    }
    TableObject::~TableObject()
    {
    }

    std::wstring TableObject::Stringify(bool outputOpCodeIfExists) const
    {
        std::wstring result = L"{";
        for (const auto &[k, v] : elements)
            result += k.Stringify(outputOpCodeIfExists) + L":" + v.Stringify(outputOpCodeIfExists) + L",";
        result = result.substr(0, result.size() - 1);
        result += L"}";
        return result;
    }
    ObjectType TableObject::Type() const
    {
        return OBJECT_TABLE;
    }

    void TableObject::Blacken(VM *vm)
    {
        Object::Blacken(vm);
        for (auto &[k, v] : elements)
        {
            k.Mark(vm);
            v.Mark(vm);
        }
    }

    bool TableObject::IsEqualTo(Object *other)
    {
        if (!IS_TABLE_OBJ(other))
            return false;

        TableObject *tableOther = TO_TABLE_OBJ(other);

        if (tableOther->elements.size() != elements.size())
            return false;

        for (const auto &[k1, v1] : elements)
        {
            bool isFound = false;
            for (const auto &[k2, v2] : tableOther->elements)
            {
                if (k1 == k2 && v1 == v2)
                    isFound = true;
            }
            if (!isFound)
                return false;
        }

        return true;
    }

    Object* TableObject::Clone() const
    {
        ValueUnorderedMap m;
        for (auto [k, v] : elements)
        {
            auto kCopy = k.Clone();
            auto vCopy = v.Clone();

            m[kCopy] = vCopy;
        }

        return new TableObject(m);
    }

    FunctionObject::FunctionObject()
        : arity(0), upValueCount(0)
    {
    }
    FunctionObject::FunctionObject(std::wstring_view name)
        : arity(0), upValueCount(0), name(name)
    {
    }
    FunctionObject::~FunctionObject()
    {
    }

    std::wstring FunctionObject::Stringify(bool outputOpCodeIfExists) const
    {
        auto result = L"<fn " + name + L":0x" + PointerAddressToString((void *)this) + L">";
        if (outputOpCodeIfExists)
            result += L"\n" + chunk.Stringify(outputOpCodeIfExists);
        return result;
    }
    ObjectType FunctionObject::Type() const
    {
        return OBJECT_FUNCTION;
    }

    void FunctionObject::Blacken(VM *vm)
    {
        Object::Blacken(vm);
        for (auto &c : chunk.constants)
            c.Mark(vm);
    }

    bool FunctionObject::IsEqualTo(Object *other)
    {
        if (!IS_FUNCTION_OBJ(other))
            return false;

        auto func = TO_FUNCTION_OBJ(other);
        if (arity != func->arity)
            return false;
        if (upValueCount != func->upValueCount)
            return false;
        if (chunk != func->chunk)
            return false;
        if (name != func->name)
            return false;
        return true;
    }

    Object* FunctionObject::Clone() const
    {
        FunctionObject* funcObj = new FunctionObject();
        funcObj->name = this->name;
        funcObj->arity = this->arity;
        funcObj->upValueCount = this->upValueCount;
        funcObj->chunk.opCodes = this->chunk.opCodes;
        funcObj->chunk.constants.resize(this->chunk.constants.size());
        for (int32_t i = 0; i < funcObj->chunk.constants.size(); ++i)
            funcObj->chunk.constants[i] = this->chunk.constants[i].Clone();
        return funcObj;
    }

    UpValueObject::UpValueObject()
    {
    }
    UpValueObject::UpValueObject(Value *location)
        : location(location)
    {
    }
    UpValueObject::~UpValueObject()
    {
    }

    std::wstring UpValueObject::Stringify(bool outputOpCodeIfExists) const
    {
        return location->Stringify(outputOpCodeIfExists);
    }
    ObjectType UpValueObject::Type() const
    {
        return OBJECT_UPVALUE;
    }

    void UpValueObject::Blacken(VM *vm)
    {
        Object::Blacken(vm);
        closed.Mark(vm);
    }

    bool UpValueObject::IsEqualTo(Object *other)
    {
        if (!IS_UPVALUE_OBJ(other))
            return false;

        auto upvo = TO_UPVALUE_OBJ(other);

        if (closed != upvo->closed)
            return false;
        if (*location != *upvo->location)
            return false;
        if (!nextUpValue->IsEqualTo(upvo->nextUpValue))
            return false;
        return true;
    }

    Object* UpValueObject::Clone() const
    {
        UpValueObject* result=new UpValueObject();
        auto tmp = location->Clone();
        result->location = &tmp;
        result->closed = closed.Clone();
        result->nextUpValue = (UpValueObject *)nextUpValue->Clone();
        return result;
    }

    ClosureObject::ClosureObject()
        : function(nullptr)
    {
    }
    ClosureObject::ClosureObject(FunctionObject *function)
        : function(function)
    {
        upvalues.resize(function->upValueCount);
    }
    ClosureObject::~ClosureObject()
    {
    }

    std::wstring ClosureObject::Stringify(bool outputOpCodeIfExists) const
    {
        return function->Stringify(outputOpCodeIfExists);
    }
    ObjectType ClosureObject::Type() const
    {
        return OBJECT_CLOSURE;
    }

    void ClosureObject::Blacken(VM *vm)
    {
        Object::Blacken(vm);
        function->Mark(vm);
        for (int32_t i = 0; i < upvalues.size(); ++i)
            if (upvalues[i])
                upvalues[i]->Mark(vm);
    }

    bool ClosureObject::IsEqualTo(Object *other)
    {
        if (!IS_CLOSURE_OBJ(other))
            return false;
        auto closure = TO_CLOSURE_OBJ(other);

        if (!function->IsEqualTo(closure->function))
            return false;
        if (upvalues.size() != closure->upvalues.size())
            return false;
        for (int32_t i = 0; i < upvalues.size(); ++i)
            if (!upvalues[i]->IsEqualTo(closure->upvalues[i]))
                return false;
        return true;
    }

    Object* ClosureObject::Clone() const
    {
        ClosureObject* result = new ClosureObject();
        result->function = (FunctionObject*)function->Clone();
        result->upvalues.resize(upvalues.size());
        for (int32_t i = 0; i < result->upvalues.size(); ++i)
            result->upvalues[i] = (UpValueObject*)upvalues[i]->Clone();
        return result;
    }

    NativeFunctionObject::NativeFunctionObject()
    {
    }
    NativeFunctionObject::NativeFunctionObject(NativeFunction f)
        : fn(f)
    {
    }
    NativeFunctionObject::~NativeFunctionObject()
    {
    }

    std::wstring NativeFunctionObject::Stringify(bool outputOpCodeIfExists) const
    {
        return L"<native function>";
    }
    ObjectType NativeFunctionObject::Type() const
    {
        return OBJECT_NATIVE_FUNCTION;
    }

    bool NativeFunctionObject::IsEqualTo(Object *other)
    {
        if (!IS_NATIVE_FUNCTION_OBJ(other))
            return false;
        return true;
    }

    Object* NativeFunctionObject::Clone() const
    {
        return new NativeFunctionObject(fn);
    }

    RefObject::RefObject(Value *pointer)
        : pointer(pointer)
    {
    }
    RefObject::~RefObject()
    {
    }

    std::wstring RefObject::Stringify(bool outputOpCodeIfExists) const
    {
        return pointer->Stringify(outputOpCodeIfExists);
    }
    ObjectType RefObject::Type() const
    {
        return OBJECT_REF;
    }

    bool RefObject::IsEqualTo(Object *other)
    {
        if (!IS_REF_OBJ(other))
            return false;
        return *pointer == *TO_REF_OBJ(other)->pointer;
    }

    Object* RefObject::Clone() const
    {
        auto tmp = pointer->Clone();
        return new RefObject(&tmp);
    }

    ClassObject::ClassObject()
    {
    }

    ClassObject::ClassObject(std::wstring_view name)
        : name(name)
    {
    }

    ClassObject::~ClassObject()
    {
    }

    std::wstring ClassObject::Stringify(bool outputOpCodeIfExists) const
    {
        std::wstring result = L"class " + name;
        if (!parents.empty())
        {
            result += L":";
            for (const auto &[k, v] : parents)
                result += k + L",";
            result = result.substr(0, result.size() - 1);
        }
        result += L"\n{\n";
        for (const auto &[k, v] : members)
            result += L"  " + k + L":" + v.Stringify(outputOpCodeIfExists) + L"\n";

        return result + L"}\n";
    }
    ObjectType ClassObject::Type() const
    {
        return OBJECT_CLASS;
    }

    void ClassObject::Blacken(VM *vm)
    {
        Object::Blacken(vm);
        for (auto &[k, v] : members)
            v.Mark(vm);
        for (auto &[k, v] : parents)
            v->Mark(vm);
    }

    bool ClassObject::IsEqualTo(Object *other)
    {
        if (!IS_CLASS_OBJ(other))
            return false;
        auto klass = TO_CLASS_OBJ(other);
        if (name != klass->name)
            return false;
        if (members != klass->members)
            return false;
        if (parents != klass->parents)
            return false;
        return true;
    }

    Object* ClassObject::Clone() const
    {
        ClassObject* classObj = new ClassObject();
        classObj->name = this->name;
        for (auto [k, v] : members)
            classObj->members[k] = v.Clone();
		for (auto [k, v] : parents)
			classObj->parents[k] = (ClassObject*)v->Clone();
        return classObj;
    }

    bool ClassObject::GetMember(const std::wstring &name, Value &retV)
    {
        auto iter = members.find(name);
        if (iter != members.end())
        {
            retV = iter->second;
            return true;
        }
        else if (!parents.empty())
        {
            bool hasValue = false;
            for (const auto &[k, v] : parents)
            {
                if (name == k)
                {
                    retV = v;
                    hasValue = true;
                }
                else
                {
                    hasValue = v->GetMember(name, retV);
                }
            }
            return hasValue;
        }
        return false;
    }

    bool ClassObject::GetParentMember(const std::wstring &name, Value &retV)
    {
        if (!parents.empty())
        {
            bool hasValue = false;
            for (const auto &[k, v] : parents)
            {
                if (name == k)
                {
                    retV = v;
                    hasValue = true;
                }
                else
                {
                    hasValue = v->GetMember(name, retV);
                }
            }
            return hasValue;
        }
        return false;
    }

    ClassClosureBindObject::ClassClosureBindObject()
    {
    }
    ClassClosureBindObject::ClassClosureBindObject(const Value &receiver, ClosureObject *cl)
        : receiver(receiver), closure(cl)
    {
    }
    ClassClosureBindObject::~ClassClosureBindObject()
    {
    }
    std::wstring ClassClosureBindObject::Stringify(bool outputOpCodeIfExists) const
    {
        return closure->Stringify(outputOpCodeIfExists);
    }
    ObjectType ClassClosureBindObject::Type() const
    {
        return OBJECT_CLASS_CLOSURE_BIND;
    }

    void ClassClosureBindObject::Blacken(VM *vm)
    {
        Object::Blacken(vm);
        receiver.Mark(vm);
        closure->Mark(vm);
    }

    bool ClassClosureBindObject::IsEqualTo(Object *other)
    {
        if (!IS_CLASS_CLOSURE_BIND_OBJ(other))
            return false;
        auto ccb = TO_CLASS_CLOSURE_BIND_OBJ(other);
        if (receiver != ccb->receiver)
            return false;
        if (!closure->IsEqualTo(ccb->closure))
            return false;
        return true;
    }

    Object* ClassClosureBindObject::Clone() const
    {
        ClassClosureBindObject* result = new ClassClosureBindObject();
        result->receiver = receiver.Clone();
        result->closure = (ClosureObject *)this->closure->Clone();
        return result;
    }

    EnumObject::EnumObject()
    {
    }
    EnumObject::EnumObject(const std::wstring &name, const std::unordered_map<std::wstring, Value> &pairs)
        : name(name), pairs(pairs)
    {
    }

    EnumObject::~EnumObject()
    {
    }

    std::wstring EnumObject::Stringify(bool outputOpCodeIfExists) const
    {
        std::wstring result = L"enum " + name + L"{";
        if (!pairs.empty())
        {
            for (const auto &[k, v] : pairs)
                result += k + L"=" + v.Stringify(outputOpCodeIfExists) + L",";
            result = result.substr(0, result.size() - 1);
        }
        return result + L"}";
    }

    ObjectType EnumObject::Type() const
    {
        return OBJECT_ENUM;
    }

    void EnumObject::Blacken(VM *vm)
    {
        Object::Blacken(vm);
        for (auto &[k, v] : pairs)
            v.Mark(vm);
    }

    bool EnumObject::GetMember(const std::wstring &name, Value &retV)
    {
        auto iter = pairs.find(name);
        if (iter != pairs.end())
        {
            retV = iter->second;
            return true;
        }
        return false;
    }

    bool EnumObject::IsEqualTo(Object *other)
    {
        if (!IS_ENUM_OBJ(other))
            return false;
        auto eo = TO_ENUM_OBJ(other);
        if (name != eo->name)
            return false;
        if (pairs != eo->pairs)
            return false;
        return true;
    }

    Object* EnumObject::Clone() const
    {
        EnumObject* enumObj = new EnumObject();
        enumObj->name = name;
        for (auto [k, v] : pairs)
            enumObj->pairs[k] = v.Clone();
        return enumObj;
    }

}