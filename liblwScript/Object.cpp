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

    std::string IntNumObject::Stringify()
    {
        return std::to_string(value);
    }
    ObjectType IntNumObject::Type()
    {
        return ObjectType::INT;
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

    std::string RealNumObject::Stringify()
    {
        return std::to_string(value);
    }
    ObjectType RealNumObject::Type()
    {
        return ObjectType::REAL;
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

    std::string BoolObject::Stringify()
    {
        return value ? "true" : "false";
    }
    ObjectType BoolObject::Type()
    {
        return ObjectType::BOOL;
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

    std::string ArrayObject::Stringify()
    {
        std::string result = "[";
        if (!elements.empty())
        {
            for (const auto &e : elements)
                result += e->Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += "]";
        return result;
    }
    ObjectType ArrayObject::Type()
    {
        return ObjectType::ARRAY;
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

    std::string TableObject::Stringify()
    {
        std::string result = "{";
        if (!elements.empty())
        {
            for (auto [key, value] : elements)
                result += key->Stringify() + ":" + value->Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += "}";
        return result;
    }
    ObjectType TableObject::Type()
    {
        return ObjectType::TABLE;
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

    FunctionObject::FunctionObject()
        : frameIndex(0)
    {
    }
    FunctionObject::FunctionObject(int64_t frameIndex)
        : frameIndex(frameIndex)
    {
    }
    FunctionObject::~FunctionObject()
    {
    }

    std::string FunctionObject::Stringify()
    {
        return "lambda";
    }
    ObjectType FunctionObject::Type()
    {
        return ObjectType::FUNCTION;
    }
    void FunctionObject::Mark()
    {
        if (marked)
            return;
        marked = true;
    }
    void FunctionObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
    }

    bool FunctionObject::IsEqualTo(Object *other)
    {
        if (!IS_FUNCTION_OBJ(other))
            return false;
        return frameIndex == TO_FUNCTION_OBJ(other)->frameIndex;
    }

    RefObject::RefObject(std::string_view address)
        : address(address)
    {
    }
    RefObject::~RefObject()
    {
    }

    std::string RefObject::Stringify()
    {
        return address;
    }
    ObjectType RefObject::Type()
    {
        return ObjectType::REF;
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
        return address == TO_REF_OBJ(other)->address;
    }

    FieldObject::FieldObject()
    {
    }
    FieldObject::FieldObject(std::string_view name,
                             const std::unordered_map<std::string, Object *> &members,
                             const std::vector<std::pair<std::string, FieldObject *>> &containedFields)
        : name(name), members(members), containedFields(containedFields)
    {
    }
    FieldObject::~FieldObject()
    {
    }

    std::string FieldObject::Stringify()
    {
        std::string result = name;

        if (!containedFields.empty())
        {
            result += ":\n";
            for (const auto &containedField : containedFields)
                result += containedField.first + ",";
            result = result.substr(0, result.size() - 1);
        }

        if (!members.empty())
        {
            result += "{\n";
            for (const auto &[key, value] : members)
                result += key + "=" + value->Stringify() + "\n";
            result = result.substr(0, result.size() - 1);
            result += "}\n";
        }
        return result;
    }
    ObjectType FieldObject::Type()
    {
        return ObjectType::FIELD;
    }
    void FieldObject::Mark()
    {
        if (marked)
            return;
        marked = true;
        for (auto [key, value] : members)
            value->Mark();
        for (auto &containedField : containedFields)
            containedField.second->Mark();
    }
    void FieldObject::UnMark()
    {
        if (!marked)
            return;
        marked = false;
        for (auto [key, value] : members)
            value->UnMark();
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
                if (key1 != key2 || !value1->IsEqualTo(value2))
                    return false;
        return true;
    }

    void FieldObject::AssignMemberByName(std::string_view name, Object *value)
    {
        auto iter = members.find(name.data());
        if (iter != members.end())
            members[name.data()] = value;
        else if (!containedFields.empty())
        {
            for (auto &containedField : containedFields)
                containedField.second->AssignMemberByName(name, value);
        }
        else
            Assert("Undefined field member:" + std::string(name));
    }

    Object *FieldObject::GetMemberByName(std::string_view name)
    {
        auto iter = members.find(name.data()); // variables in self scope
        if (iter != members.end())
            return iter->second;
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

    Object *FieldObject::GetMemberByAddress(std::string_view address)
    {
        if (!members.empty())
        {
            for (auto [key, value] : members)
                if (PointerAddressToString(value) == address)
                    return value;
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

        Assert("No Object's address:" + std::string(address) + "in field:" + std::string(name.data()));
        return nullptr;
    }
}