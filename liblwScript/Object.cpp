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
        auto result= L"<fn " + name +L":0x"+PointerAddressToString((void*)this)+ L">\n";
        result+=chunk.Stringify();
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
}