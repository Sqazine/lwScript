#pragma once
#include <string>
#include <vector>
namespace lws
{
#define IS_STR_OBJ(obj) (obj->Type() == OBJECT_STR)
#define IS_ARRAY_OBJ(obj) (obj->Type() == OBJECT_ARRAY)

#define TO_STR_OBJ(obj) ((lws::StrObject *)obj)
#define TO_ARRAY_OBJ(obj) ((lws::ArrayObject *)obj)

    enum ObjectType
    {
        OBJECT_STR,
        OBJECT_ARRAY,
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

    struct ArrayObject : public Object
    {
        ArrayObject();
        ArrayObject(const std::vector<struct Value> &elements);
        ~ArrayObject();

        std::wstring Stringify() const override;
        ObjectType Type() const override;
        void Mark() override;
        void UnMark() override;

        bool IsEqualTo(Object *other) override;

        std::vector<struct Value> elements;
    };
}