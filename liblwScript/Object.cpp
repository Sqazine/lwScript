#include "Object.h"

namespace lws
{

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
            for (const auto& [key, value] : elements)
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

        for (const auto& [key, value] : elements)
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
                if (key1!=key2 || value1!=value2)
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

    RefObject::RefObject(std::wstring_view name, const Value& index)
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

    FieldObject::FieldObject()
    {
    }
    FieldObject::FieldObject(std::wstring_view name,
                             const std::unordered_map<std::wstring, ValueDesc> &members,
                             const std::vector<std::pair<std::wstring, FieldObject *>> &containedFields)
        : name(name), members(members), containedFields(containedFields)
    {
    }
    FieldObject::~FieldObject()
    {
    }

    std::wstring FieldObject::Stringify()
    {
        std::wstring result = L"instance of field:\n" + name;

        if (!containedFields.empty())
        {
            result += L":";
            for (const auto &containedField : containedFields)
                result += containedField.first + L",";
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
    ObjectType FieldObject::Type()
    {
        return OBJECT_FIELD;
    }
    void FieldObject::Mark()
    {
        if (marked)
            return;
        marked = true;
        for (const auto& [memberKey, memberValue] : members)
            memberValue.value.Mark();
        for (auto &containedField : containedFields)
            containedField.second->Mark();
    }
    void FieldObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
        for (const auto &[memberKey, memberValue] : members)
            memberValue.value.UnMark();
        for (auto &containedField : containedFields)
            containedField.second->UnMark();
    }
    bool FieldObject::IsEqualTo(Object *other)
    {
        if (!IS_FIELD_OBJ(other))
            return false;

        if (name != TO_FIELD_OBJ(other)->name)
            return false;

        for (const auto& [key1, value1] : members)
            for (const auto& [key2, value2] : TO_FIELD_OBJ(other)->members)
                if (key1 != key2 || value1!=value2)
                    return false;
        return true;
    }

    void FieldObject::AssignMemberByName(std::wstring_view name, const Value& value)
    {
        auto iter = members.find(name.data());
        if (iter != members.end())
        {
            if (members[name.data()].type != ValueDescType::CONST)
                members[name.data()].value = value;
            else
                Assert(L"The member:" + std::wstring(name) + L" in the field:" + this->name + L" is a const member,cannot be reassigned.");
        }
        else if (!containedFields.empty())
        {
            for (auto &containedField : containedFields)
                containedField.second->AssignMemberByName(name, value);
        }
        else
            Assert(L"Undefined field member:" + std::wstring(name));
    }

    Value FieldObject::GetMemberByName(std::wstring_view name)
    {
        auto iter = members.find(name.data()); // variables in self scope
        if (iter != members.end())
            return iter->second.value;
        else // variables in contained field
        {
            for (const auto &containedField : containedFields)
            {
                // the contained field self
                if (containedField.first == name)
                    return containedField.second;
                else // the member
                {
                    auto member = containedField.second->GetMemberByName(name);
                    if (!IS_INVALID_VALUE(member))
                        return member;
                }
            }
        }
        return gInvalidValue;
    }
    Value FieldObject::GetMemberByAddress(std::wstring_view address)
    {
        if (!members.empty())
        {
            for (auto& [memberKey, memberValue] : members)
                if (PointerAddressToString(&memberValue.value) == address)
                    return memberValue.value;
        }

        if (!containedFields.empty()) // in contained field
        {
            for (const auto &containedField : containedFields)
            {
                // the contained field self
                if (PointerAddressToString(containedField.second) == address)
                    return containedField.second;
                else // the member in contained field
                {
                    auto member = containedField.second->GetMemberByAddress(address);
                    if (!IS_INVALID_VALUE(member))
                        return member;
                }
            }
        }

        Assert(L"No Object's address:" + std::wstring(address) + L"in field:" + std::wstring(name.data()));
        return gInvalidValue;
    }
}