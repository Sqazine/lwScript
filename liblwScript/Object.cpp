#include "Object.h"

namespace lws
{

    StrObject::StrObject()
    {
    }
    StrObject::StrObject(std::wstring_view value)
        : value(value)
    {
    }
    StrObject::~StrObject()
    {
    }

    std::wstring StrObject::Stringify()
    {
        return value;
    }
    ObjectType StrObject::Type()
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

    std::wstring ArrayObject::Stringify()
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
    ObjectType ArrayObject::Type()
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
        ValueUnorderedMap().swap(elements);
    }

    std::wstring TableObject::Stringify()
    {
        std::wstring result = L"{";
        if (!elements.empty())
        {
            for (const auto &[key, value] : elements)
                result += key.Stringify() + L":" + value.Stringify() + L",";
            result = result.substr(0, result.size() - 1);
        }
        result += L"}";
        return result;
    }
    ObjectType TableObject::Type()
    {
        return OBJECT_TABLE;
    }
    void TableObject::Mark()
    {
        if (marked)
            return;
        marked = true;

        for (const auto &[key, value] : elements)
        {
            key.Mark();
            value.Mark();
        }
    }
    void TableObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;

        for (auto [key, value] : elements)
        {
            key.UnMark();
            value.UnMark();
        }
    }

    bool TableObject::IsEqualTo(Object *other)
    {
        if (!IS_TABLE_OBJ(other))
            return false;

        TableObject *tableOther = TO_TABLE_OBJ(other);

        if (tableOther->elements.size() != elements.size())
            return false;

        for (auto [key1, value1] : elements)
            for (auto [key2, value2] : tableOther->elements)
                if (key1 != key2 || value1 != value2)
                    return false;

        return true;
    }

    LambdaObject::LambdaObject()
        : frameIndex(0)
    {
    }
    LambdaObject::LambdaObject(int64_t frameIndex)
        : frameIndex(frameIndex)
    {
    }
    LambdaObject::~LambdaObject()
    {
    }

    std::wstring LambdaObject::Stringify()
    {
        return L"lambda function";
    }
    ObjectType LambdaObject::Type()
    {
        return OBJECT_LAMBDA;
    }
    void LambdaObject::Mark()
    {
        if (marked)
            return;
        marked = true;
    }
    void LambdaObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
    }

    bool LambdaObject::IsEqualTo(Object *other)
    {
        if (!IS_LAMBDA_OBJ(other))
            return false;
        return frameIndex == TO_LAMBDA_OBJ(other)->frameIndex;
    }

    RefObject::RefObject(std::wstring_view name, const Value &index)
        : name(name), index(index), isAddressReference(false)
    {
    }

    RefObject::RefObject(std::wstring_view address)
        : isAddressReference(true), address(address)
    {
    }
    RefObject::~RefObject()
    {
    }

    std::wstring RefObject::Stringify()
    {
        if (!isAddressReference)
        {
            std::wstring result = name;
            if (!IS_INVALID_VALUE(index))
                result += L"[" + index.Stringify() + L"]";
            return result;
        }
        else
            return L"&" + address;
    }
    ObjectType RefObject::Type()
    {
        return OBJECT_REF;
    }
    void RefObject::Mark()
    {
        if (marked)
            return;
        marked = true;
    }
    void RefObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
    }
    bool RefObject::IsEqualTo(Object *other)
    {
        if (!IS_REF_OBJ(other))
            return false;
        if (isAddressReference != TO_REF_OBJ(other)->isAddressReference)
            return false;
        else if (!isAddressReference)
            return name == TO_REF_OBJ(other)->name;
        else
            return address == TO_REF_OBJ(other)->address;
    }

    ClassObject::ClassObject()
    {
    }
    ClassObject::ClassObject(std::wstring_view name,
                             const std::unordered_map<std::wstring, ValueDesc> &members,
                             const std::vector<std::pair<std::wstring, ClassObject *>> &parentClasses)
        : name(name), members(members), parentClasses(parentClasses)
    {
    }
    ClassObject::~ClassObject()
    {
    }

    std::wstring ClassObject::Stringify()
    {
        std::wstring result = L"instance of class:\n" + name;

        if (!parentClasses.empty())
        {
            result += L":";
            for (const auto &parentClass : parentClasses)
                result += parentClass.first + L",";
            result = result.substr(0, result.size() - 1);
        }

        if (!members.empty())
        {
            result += L"\n{\n";
            for (const auto &[key, value] : members)
            {
                if (value.type == ValueDescType::CONST)
                    result += L"    const  " + key + L"\n";
                else
                    result += L"    let  " + key + L"\n";
            }
            result = result.substr(0, result.size() - 1);
            result += L"\n}";
        }
        return result;
    }
    ObjectType ClassObject::Type()
    {
        return OBJECT_CLASS;
    }
    void ClassObject::Mark()
    {
        if (marked)
            return;
        marked = true;
        for (const auto &[memberKey, memberValue] : members)
            memberValue.value.Mark();
        for (auto &parentClass : parentClasses)
            parentClass.second->Mark();
    }
    void ClassObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
        for (const auto &[memberKey, memberValue] : members)
            memberValue.value.UnMark();
        for (auto &parentClass : parentClasses)
            parentClass.second->UnMark();
    }
    bool ClassObject::IsEqualTo(Object *other)
    {
        if (!IS_CLASS_OBJ(other))
            return false;

        if (name != TO_CLASS_OBJ(other)->name)
            return false;

        for (const auto &[key1, value1] : members)
            for (const auto &[key2, value2] : TO_CLASS_OBJ(other)->members)
                if (key1 != key2 || value1 != value2)
                    return false;
        return true;
    }

    void ClassObject::AssignMemberByName(std::wstring_view name, const Value &value)
    {
        auto iter = members.find(name.data());
        if (iter != members.end())
        {
            if (members[name.data()].type != ValueDescType::CONST)
                members[name.data()].value = value;
            else
                ASSERT(L"The member:" + std::wstring(name) + L" in the class:" + this->name + L" is a const member,cannot be reassigned.")
        }
        else if (!parentClasses.empty())
        {
            for (auto &parentClass : parentClasses)
                parentClass.second->AssignMemberByName(name, value);
        }
        else
            ASSERT(L"Undefined class member:" + std::wstring(name))
    }

    Value ClassObject::GetMemberByName(std::wstring_view name)
    {
        auto iter = members.find(name.data()); // variables in self scope
        if (iter != members.end())
            return iter->second.value;
        else // variables in parent class
        {
            for (const auto &parentClass : parentClasses)
            {
                // the parent class self
                if (parentClass.first == name)
                    return parentClass.second;
                else // the member
                {
                    auto member = parentClass.second->GetMemberByName(name);
                    if (!IS_INVALID_VALUE(member))
                        return member;
                }
            }
        }
        return gInvalidValue;
    }
    Value ClassObject::GetMemberByAddress(std::wstring_view address)
    {
        if (!members.empty())
        {
            for (auto &[memberKey, memberValue] : members)
                if (PointerAddressToString(memberValue.value.object) == address)
                    return memberValue.value;
        }

        if (!parentClasses.empty()) // in parent class
        {
            for (const auto &parentClass : parentClasses)
            {
                // the parent class self
                if (PointerAddressToString(parentClass.second) == address)
                    return parentClass.second;
                else // the member in parent class
                {
                    auto member = parentClass.second->GetMemberByAddress(address);
                    if (!IS_INVALID_VALUE(member))
                        return member;
                }
            }
        }

        ASSERT(L"No Object's address:" + std::wstring(address) + L"in class:" + std::wstring(name.data()))
        return gInvalidValue;
    }
}