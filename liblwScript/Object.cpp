#include "Object.h"
#include "Value.h"
#include "Chunk.h"
#include "Utils.h"
namespace lws
{

    Object::Object()
        : marked(false), next(nullptr)
    {
    }
    Object::~Object()
    {
    }

    void Object::Mark()
    {
        if (marked)
            return;

        marked = true;
    }
    void Object ::UnMark()
    {
        if (!marked)
            return;

        marked = false;
    }

    StrObject::StrObject(std::wstring_view value)
        : value(value)
    {
    }
    StrObject::~StrObject()
    {
    }
    std::wstring StrObject::Stringify() const
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

    std::wstring ArrayObject::Stringify() const
    {
        std::wstring result = L"[";
        if (!elements.empty())
        {
            for (const auto &e : elements)
                result += e.Stringify() + L",";
            result = result.substr(0, result.size() - 1);
        }
        result += L"]";
        return result;
    }
    ObjectType ArrayObject::Type() const
    {
        return OBJECT_ARRAY;
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

    std::wstring TableObject::Stringify() const
    {
        std::wstring result = L"{";
        for (const auto &[k, v] : elements)
            result += k.Stringify() + L":" + v.Stringify() + L",";
        result = result.substr(0, result.size() - 1);
        result += L"}";
        return result;
    }
    ObjectType TableObject::Type() const
    {
        return OBJECT_TABLE;
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

    std::wstring FunctionObject::Stringify() const
    {
        auto result = L"<fn " + name + L":0x" + PointerAddressToString((void *)this) + L">";
#ifdef _DEBUG
        result += L"\n" + chunk.Stringify();
#endif
        return result;
    }
    ObjectType FunctionObject::Type() const
    {
        return OBJECT_FUNCTION;
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

    std::wstring UpValueObject::Stringify() const
    {
        return location->Stringify();
    }
    ObjectType UpValueObject::Type() const
    {
        return OBJECT_UPVALUE;
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

    std::wstring ClosureObject::Stringify() const
    {
        return function->Stringify();
    }
    ObjectType ClosureObject::Type() const
    {
        return OBJECT_CLOSURE;
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

    std::wstring NativeFunctionObject::Stringify() const
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

    RefObject::RefObject(Value *pointer)
        : pointer(pointer)
    {
    }
    RefObject::~RefObject()
    {
    }

    std::wstring RefObject::Stringify() const
    {
        return pointer->Stringify();
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

    std::wstring ClassObject::Stringify() const
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
            result += k + L":" + v.Stringify() + L"\n";

        return result + L"}\n";
    }
    ObjectType ClassObject::Type() const
    {
        return OBJECT_CLASS;
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
    std::wstring ClassClosureBindObject::Stringify() const
    {
        return closure->Stringify();
    }
    ObjectType ClassClosureBindObject::Type() const
    {
        return OBJECT_CLASS_CLOSURE_BIND;
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

    std::wstring EnumObject::Stringify() const
    {
        std::wstring result = L"enum " + name + L"{";
        if (!pairs.empty())
        {
            for (const auto &[k, v] : pairs)
                result += k + L"=" + v.Stringify() + L",";
            result = result.substr(0, result.size() - 1);
        }
        return result + L"}";
    }
    ObjectType EnumObject::Type() const
    {
        return OBJECT_ENUM;
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

}