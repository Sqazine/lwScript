#include "Value.h"
#include "Object.h"
#include "VM.h"
namespace lwscript
{
    Value::Value()
        : kind(ValueKind::NIL), object(nullptr)
    {
    }
    Value::Value(double number)
        : realnum(number), kind(ValueKind::REAL)
    {
    }

    Value::Value(int64_t integer)
        : integer(integer), kind(ValueKind::INT)
    {
    }
    Value::Value(bool boolean)
        : boolean(boolean), kind(ValueKind::BOOL)
    {
    }

    Value::Value(Object *object)
        : object(object), kind(ValueKind::OBJECT)
    {
    }

    STD_STRING Value::ToString() const
    {
        switch (kind)
        {
        case ValueKind::INT:
            return TO_STRING(integer);
        case ValueKind::REAL:
            return TO_STRING(realnum);
        case ValueKind::BOOL:
            return boolean ? TEXT("true") : TEXT("false");
        case ValueKind::NIL:
            return TEXT("null");
        case ValueKind::OBJECT:
            return object->ToString();
        default:
            return TEXT("null");
        }
        return TEXT("null");
    }
    void Value::Mark() const
    {
        if (kind == ValueKind::OBJECT)
            object->Mark();
    }
    void Value::UnMark() const
    {
        if (kind == ValueKind::OBJECT)
            object->UnMark();
    }

    Value Value::Clone() const
    {
        Value result;
        result.kind = kind;
        switch (kind)
        {
        case ValueKind::INT:
            result.integer = integer;
            break;
        case ValueKind::REAL:
            result.realnum = realnum;
            break;
        case ValueKind::BOOL:
            result.boolean = boolean;
            break;
        case ValueKind::NIL:
            break;
        case ValueKind::OBJECT:
            result.object = object->Clone();
            break;
        default:
            break;
        }
        return result;
    }

    std::vector<uint8_t> Value::Serialize() const
    {
        std::vector<uint8_t> result;

        result.emplace_back(kind);
        result.emplace_back(privilege);

        if (IS_INT_VALUE(*this))
        {
            result.emplace_back(uint8_t((integer & 0xFF00000000000000) >> 56));
            result.emplace_back(uint8_t((integer & 0x00FF000000000000) >> 48));
            result.emplace_back(uint8_t((integer & 0x0000FF0000000000) >> 40));
            result.emplace_back(uint8_t((integer & 0x000000FF00000000) >> 32));
            result.emplace_back(uint8_t((integer & 0x00000000FF000000) >> 24));
            result.emplace_back(uint8_t((integer & 0x0000000000FF0000) >> 16));
            result.emplace_back(uint8_t((integer & 0x000000000000FF00) >> 8));
            result.emplace_back(uint8_t((integer & 0x00000000000000FF) >> 0));
        }

        return result;
    }

    void Value::Deserialize(const std::vector<uint8_t> &data)
    {
        kind = (ValueKind)data[0];
        privilege = (Privilege)data[1];

        if (IS_INT_VALUE(*this))
        {
            integer = ((data[2] & 0x00000000000000FF) << 56) |
                      ((data[3] & 0x00000000000000FF) << 48) |
                      ((data[4] & 0x00000000000000FF) << 40) |
                      ((data[5] & 0x00000000000000FF) << 32) |
                      ((data[6] & 0x00000000000000FF) << 24) |
                      ((data[7] & 0x00000000000000FF) << 16) |
                      ((data[8] & 0x00000000000000FF) << 8) |
                      ((data[9] & 0x00000000000000FF) << 0);
        }
    }

    bool operator==(const Value &left, const Value &right)
    {
        switch (left.kind)
        {
        case ValueKind::INT:
        {
            if (IS_INT_VALUE(right))
                return TO_INT_VALUE(left) == TO_INT_VALUE(right);
            else if (IS_REAL_VALUE(right))
                return TO_INT_VALUE(left) == TO_REAL_VALUE(right);
            else
                return false;
        }
        case ValueKind::REAL:
        {
            if (IS_INT_VALUE(right))
                return TO_REAL_VALUE(left) == TO_INT_VALUE(right);
            else if (IS_REAL_VALUE(right))
                return TO_REAL_VALUE(left) == TO_REAL_VALUE(right);
            else
                return false;
        }
        case ValueKind::NIL:
            return IS_NULL_VALUE(right);
        case ValueKind::BOOL:
        {
            if (IS_BOOL_VALUE(right))
                return TO_BOOL_VALUE(left) == TO_BOOL_VALUE(right);
            else
                return false;
        }
        case ValueKind::OBJECT:
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

    size_t ValueHash::operator()(const Value *v) const
    {
        switch (v->kind)
        {
        case ValueKind::NIL:
            return std::hash<ValueKind>()(v->kind);
        case ValueKind::INT:
            return std::hash<ValueKind>()(v->kind) ^ std::hash<int64_t>()(v->integer);
        case ValueKind::REAL:
            return std::hash<ValueKind>()(v->kind) ^ std::hash<double>()(v->realnum);
        case ValueKind::BOOL:
            return std::hash<ValueKind>()(v->kind) ^ std::hash<bool>()(v->boolean);
        case ValueKind::OBJECT:
            return std::hash<ValueKind>()(v->kind) ^ std::hash<Object *>()(v->object);
        default:
            return std::hash<ValueKind>()(v->kind);
        }
    }

    size_t ValueHash::operator()(const Value &v) const
    {
        return ValueHash()(&v);
    }

    size_t HashValueList(Value *start, size_t count)
    {
        std::size_t seed = count;
        for (size_t i = 0; i < count; ++i)
            seed ^= ValueHash()((start + i)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

    size_t HashValueList(Value *start, Value *end)
    {
        return HashValueList(start, end - start);
    }
}