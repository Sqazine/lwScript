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