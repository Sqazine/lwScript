#include "Object.h"

namespace lws
{
    IntNumObject::IntNumObject()
        : value(0)
    {
    }
    IntNumObject::IntNumObject(int64_t value)
        : value(value)
    {
    }
    IntNumObject::~IntNumObject()
    {
    }

    std::wstring IntNumObject::Stringify()
    {
        return std::to_wstring(value);
    }
    ObjectType IntNumObject::Type()
    {
        return OBJECT_INT;
    }
    void IntNumObject::Mark()
    {
        if (marked)
            return;
        marked = true;
    }
    void IntNumObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
    }
    bool IntNumObject::IsEqualTo(Object *other)
    {
        if (!IS_INT_OBJ(other))
            return false;
        return value == TO_INT_OBJ(other)->value;
    }

    RealNumObject::RealNumObject()
        : value(0.0)
    {
    }
    RealNumObject::RealNumObject(double value)
        : value(value)
    {
    }
    RealNumObject::~RealNumObject()
    {
    }

    std::wstring RealNumObject::Stringify()
    {
        return std::to_wstring(value);
    }
    ObjectType RealNumObject::Type()
    {
        return OBJECT_REAL;
    }
    void RealNumObject::Mark()
    {
        if (marked)
            return;
        marked = true;
    }
    void RealNumObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
    }
    bool RealNumObject::IsEqualTo(Object *other)
    {
        if (!IS_REAL_OBJ(other))
            return false;
        return value == TO_REAL_OBJ(other)->value;
    }

    StrObject::StrObject() {}
    StrObject::StrObject(std::wstring_view value) : value(value) {}
    StrObject::~StrObject() {}

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

    BoolObject::BoolObject()
        : value(false)
    {
    }
    BoolObject::BoolObject(bool value)
        : value(value)
    {
    }
    BoolObject::~BoolObject()
    {
    }

    std::wstring BoolObject::Stringify()
    {
        return value ? L"true" : L"false";
    }
    ObjectType BoolObject::Type()
    {
        return OBJECT_BOOL;
    }
    void BoolObject::Mark()
    {
        if (marked)
            return;
        marked = true;
    }
    void BoolObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
    }
    bool BoolObject::IsEqualTo(Object *other)
    {
        if (!IS_BOOL_OBJ(other))
            return false;
        return value == TO_BOOL_OBJ(other)->value;
    }

    NullObject::NullObject()
    {
    }
    NullObject::~NullObject()
    {
    }

    std::wstring NullObject::Stringify()
    {
        return L"null";
    }
    ObjectType NullObject::Type()
    {
        return OBJECT_NULL;
    }
    void NullObject::Mark()
    {
        marked = true;
    }
    void NullObject::UnMark()
    {
        marked = false;
    }
    bool NullObject::IsEqualTo(Object *other)
    {
        if (!IS_NULL_OBJ(other))
            return false;
        return true;
    }

    ArrayObject::ArrayObject()
    {
    }
    ArrayObject::ArrayObject(const std::vector<Object *> &elements)
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
                result += e->Stringify() + L",";
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
            e->Mark();
    }
    void ArrayObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;

        for (const auto &e : elements)
            e->UnMark();
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
    TableObject::TableObject(const std::unordered_map<Object *, Object *> &elements)
        : elements(elements)
    {
    }
    TableObject::~TableObject()
    {
        std::unordered_map<Object *, Object *>().swap(elements);
    }

    std::wstring TableObject::Stringify()
    {
        std::wstring result = L"{";
        if (!elements.empty())
        {
            for (auto [key, value] : elements)
                result += key->Stringify() + L":" + value->Stringify() + L",";
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

        for (auto [key, value] : elements)
        {
            key->Mark();
            value->Mark();
        }
    }
    void TableObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;

        for (auto [key, value] : elements)
        {
            key->UnMark();
            value->UnMark();
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
                if (!key1->IsEqualTo(key2) || !value1->IsEqualTo(value2))
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

    RefVarObject::RefVarObject(std::wstring_view name, Object *index)
        : name(name), index(index)
    {
    }
    RefVarObject::~RefVarObject()
    {
    }

    std::wstring RefVarObject::Stringify()
    {
        std::wstring result = name;
        if (index)
            result += L"[" + index->Stringify() + L"]";
        return result;
    }
    ObjectType RefVarObject::Type()
    {
        return OBJECT_REF_VAR;
    }
    void RefVarObject::Mark()
    {
        if (marked)
            return;
        marked = true;
    }
    void RefVarObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
    }
    bool RefVarObject::IsEqualTo(Object *other)
    {
        if (!IS_REF_VAR_OBJ(other))
            return false;
        return name == TO_REF_VAR_OBJ(other)->name;
    }

    FieldObject::FieldObject()
    {
    }
    FieldObject::FieldObject(std::wstring_view name,
                             const std::unordered_map<std::wstring, ObjectDesc> &members,
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
                if (value.type == ObjectDescType::CONST)
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
        for (auto [key, value] : members)
            value.object->Mark();
        for (auto &containedField : containedFields)
            containedField.second->Mark();
    }
    void FieldObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
        for (auto [key, value] : members)
            value.object->UnMark();
        for (auto &containedField : containedFields)
            containedField.second->UnMark();
    }
    bool FieldObject::IsEqualTo(Object *other)
    {
        if (!IS_FIELD_OBJ(other))
            return false;

        if (name != TO_FIELD_OBJ(other)->name)
            return false;

        for (auto [key1, value1] : members)
            for (auto [key2, value2] : TO_FIELD_OBJ(other)->members)
                if (key1 != key2 || !value1.object->IsEqualTo(value2.object))
                    return false;
        return true;
    }

    void FieldObject::AssignMemberByName(std::wstring_view name, Object *value)
    {
        auto iter = members.find(name.data());
        if (iter != members.end())
        {
            if (members[name.data()].type != ObjectDescType::CONST)
                members[name.data()].object = value;
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

    Object *FieldObject::GetMemberByName(std::wstring_view name)
    {
        auto iter = members.find(name.data()); // variables in self scope
        if (iter != members.end())
            return iter->second.object;
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
                    if (member)
                        return member;
                }
            }
        }
        return nullptr;
    }
    Object *FieldObject::GetMemberByAddress(std::wstring_view address)
    {
        if (!members.empty())
        {
            for (auto [key, value] : members)
                if (PointerAddressToString(value.object) == address)
                    return value.object;
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
                    if (member)
                        return member;
                }
            }
        }

        Assert(L"No Object's address:" + std::wstring(address) + L"in field:" + std::wstring(name.data()));
        return nullptr;
    }
    RefObjObject::RefObjObject(std::wstring_view address)
        : address(address)
    {
    }
    RefObjObject::~RefObjObject()
    {
    }
    std::wstring RefObjObject::Stringify()
    {
        return L"&" + address;
    }
    ObjectType RefObjObject::Type()
    {
        return OBJECT_REF_OBJ;
    }
    void RefObjObject::Mark()
    {
        marked = true;
    }
    void RefObjObject::UnMark()
    {
        marked = false;
    }
    bool RefObjObject::IsEqualTo(Object *other)
    {
        if (!IS_REF_OBJ_OBJ(other))
            return false;
        return address == TO_REF_OBJ_OBJ(other)->address;
    }
}