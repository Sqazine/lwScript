#include "Value.h"
#include "Object.h"
#include "VM.h"
namespace lwscript
{
    Value::Value()
        : type(VALUE_NULL), object(nullptr)
    {
    }
    Value::Value(double number)
        : realnum(number), type(VALUE_REAL)
    {
    }

    Value::Value(int64_t integer)
        : integer(integer), type(VALUE_INT)
    {
    }
    Value::Value(bool boolean)
        : boolean(boolean), type(VALUE_BOOL)
    {
    }

    Value::Value(Object *object)
        : object(object), type(VALUE_OBJECT)
    {
    }

    Value::~Value()
    {
    }

    std::wstring Value::ToString() const
    {
        switch (type)
        {
        case VALUE_INT:
            return std::to_wstring(integer);
        case VALUE_REAL:
            return std::to_wstring(realnum);
        case VALUE_BOOL:
            return boolean ? L"true" : L"false";
        case VALUE_NULL:
            return L"null";
        case VALUE_OBJECT:
            return object->ToString();
        default:
            return L"null";
        }
        return L"null";
    }
    void Value::Mark(VM *vm) const
    {
        if (type == VALUE_OBJECT)
            object->Mark(vm);
    }
    void Value::UnMark() const
    {
        if (type == VALUE_OBJECT)
            object->UnMark();
    }

    Value Value::Clone() const
    {
        Value result;
        result.type = this->type;
        switch (type)
        {
        case VALUE_INT:
            result.integer = this->integer;
            break;
        case VALUE_REAL:
            result.realnum = this->realnum;
            break;
        case VALUE_BOOL:
            result.boolean = this->boolean;
            break;
        case VALUE_NULL:
            break;
        case VALUE_OBJECT:
            result.object = this->object->Clone();
            break;
        default:
            break;
        }
        return result;
    }

    bool operator==(const Value &left, const Value &right)
    {
        switch (left.type)
        {
        case VALUE_INT:
        {
            if (IS_INT_VALUE(right))
                return TO_INT_VALUE(left) == TO_INT_VALUE(right);
            else if (IS_REAL_VALUE(right))
                return TO_INT_VALUE(left) == TO_REAL_VALUE(right);
            else
                return false;
        }
        case VALUE_REAL:
        {
            if (IS_INT_VALUE(right))
                return TO_REAL_VALUE(left) == TO_INT_VALUE(right);
            else if (IS_REAL_VALUE(right))
                return TO_REAL_VALUE(left) == TO_REAL_VALUE(right);
            else
                return false;
        }
        case VALUE_NULL:
            return IS_NULL_VALUE(right);
        case VALUE_BOOL:
        {
            if (IS_BOOL_VALUE(right))
                return TO_BOOL_VALUE(left) == TO_BOOL_VALUE(right);
            else
                return false;
        }
        case VALUE_OBJECT:
        {
            if (IS_OBJECT_VALUE(right))
                return TO_OBJECT_VALUE(left)->IsEqualTo(TO_OBJECT_VALUE(right));
            else
                return false;
        }
        default:
            return false;
        }
        return false;
    }

    bool operator!=(const Value &left, const Value &right)
    {
        return !(left == right);
    }

    size_t ValueHash::operator()(const Value &v) const
    {
        switch (v.type)
        {
        case VALUE_NULL:
            return std::hash<ValueType>()(v.type);
        case VALUE_INT:
            return std::hash<ValueType>()(v.type) ^ std::hash<int64_t>()(v.integer);
        case VALUE_REAL:
            return std::hash<ValueType>()(v.type) ^ std::hash<double>()(v.realnum);
        case VALUE_BOOL:
            return std::hash<ValueType>()(v.type) ^ std::hash<bool>()(v.boolean);
        case VALUE_OBJECT:
            return std::hash<ValueType>()(v.type) ^ std::hash<Object *>()(v.object);
        default:
            return std::hash<ValueType>()(v.type);
        }
    }

    size_t ValueVecHash::operator()(const std::vector<Value> &vec) const
    {
        std::size_t seed = vec.size();
        for (auto &v : vec)
        {
            seed ^= ValueHash()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
}