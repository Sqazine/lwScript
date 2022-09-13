#include "Object.h"

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
        return STR_OBJECT;
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
}