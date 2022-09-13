#pragma once
#include <string>
namespace lws
{
#define IS_STR_OBJ(obj) (obj->Type() == STR_OBJECT)

#define TO_STR_OBJ(obj) ((lws::StrObject *)obj)

    enum ObjectType
    {
        STR_OBJECT,
    };

    struct Object
    {
        Object() : marked(false), next(nullptr) {}
        virtual ~Object() {}

        virtual std::wstring Stringify() const = 0;
        virtual ObjectType Type() const = 0;
        virtual void Mark() = 0;
        virtual void UnMark() = 0;
        virtual bool IsEqualTo(Object *other) = 0;

        bool marked;
        Object *next;
    };

    struct StrObject : public Object
    {
        StrObject(std::wstring_view value);
        ~StrObject();

        std::wstring Stringify() const override;
        ObjectType Type() const override;
        void Mark() override;
        void UnMark() override;
        bool IsEqualTo(Object *other) override;

        std::wstring value;
    };
}