#include "Object.h"
#include "Value.h"
#include "Chunk.h"
#include "Utils.h"
namespace lws
{

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
    void StrObject::Mark()
    {
        marked = true;
    }
    void StrObject::UnMark()
    {
        marked = false;
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
    void ArrayObject::Mark()
    {
        if (marked)
            return;
        marked = true;

        for (const auto &e : elements)
            e.Mark();
    }
    void ArrayObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;

        for (const auto &e : elements)
            e.UnMark();
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
    void TableObject::Mark()
    {
        if (marked)
            return;
        marked = true;

        for (const auto &[k, v] : elements)
        {
            k.Mark();
            v.Mark();
        }
    }
    void TableObject::UnMark()
    {
        if (!marked)
            return;
        marked = true;

        for (const auto &[k, v] : elements)
        {
            k.UnMark();
            v.UnMark();
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
    void FunctionObject::Mark()
    {
        marked = true;
    }
    void FunctionObject::UnMark()
    {
        marked = false;
    }

    bool FunctionObject::IsEqualTo(Object *other)
    {
        return false;
    }

    UpValueObject::UpValueObject()
    {
    }
    UpValueObject::UpValueObject(Value *location)
    :location(location)
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
    void UpValueObject::Mark()
    {
        marked=true;
    }
    void UpValueObject::UnMark()
    {
        marked=false;
    }

    bool UpValueObject::IsEqualTo(Object *other)
    {
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
    void ClosureObject::Mark()
    {
        marked = true;
    }
    void ClosureObject::UnMark()
    {
        marked = false;
    }

    bool ClosureObject::IsEqualTo(Object *other)
    {
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
    void NativeFunctionObject::Mark()
    {
        marked = true;
    }
    void NativeFunctionObject::UnMark()
    {
        marked = false;
    }

    bool NativeFunctionObject::IsEqualTo(Object *other)
    {
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
    void RefObject::Mark()
    {
        marked = true;
    }
    void RefObject::UnMark()
    {
        marked = false;
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
    void ClassObject::Mark()
    {
        marked = true;
    }
    void ClassObject::UnMark()
    {
        marked = false;
    }
    bool ClassObject::IsEqualTo(Object *other)
    {
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
    void ClassClosureBindObject::Mark()
    {
        marked = true;
    }
    void ClassClosureBindObject::UnMark()
    {
        marked = false;
    }
    bool ClassClosureBindObject::IsEqualTo(Object *other)
    {
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
    void EnumObject::Mark()
    {
        marked = true;
    }
    void EnumObject::UnMark()
    {
        marked = false;
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
        return true;
    }

}