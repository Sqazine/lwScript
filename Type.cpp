#include "Type.h"

namespace lwscript
{
    constexpr struct
    {
        STD_STRING_VIEW name;
        TypeKind kind;
    } primitiveTypeMap[] = {
        {TEXT("i8"), TypeKind::I8},
        {TEXT("u8"), TypeKind::U8},
        {TEXT("i16"), TypeKind::I16},
        {TEXT("u16"), TypeKind::U16},
        {TEXT("i32"), TypeKind::I32},
        {TEXT("u32"), TypeKind::U32},
        {TEXT("i64"), TypeKind::I64},
        {TEXT("u64"), TypeKind::U64},
        {TEXT("f32"), TypeKind::F32},
        {TEXT("f64"), TypeKind::F64},
        {TEXT("bool"), TypeKind::BOOL},
        {TEXT("char"), TypeKind::CHAR},
        {TEXT("any"), TypeKind::ANY},
        {TEXT("string"), TypeKind::STRING},
    };

    Type::Type() noexcept
        : mKind(TypeKind::UNDEFINED), mCategory(TypeCategory::PRIMITIVE), mName(TEXT("undefined"))
    {
    }

    Type::Type(STD_STRING_VIEW name, const SourceLocation &scl) noexcept
    {
        for (const auto &p : primitiveTypeMap)
        {
            if (p.name == name)
            {
                mName = name;
                mKind = p.kind;
                mCategory = TypeCategory::PRIMITIVE;
                mSourceLocation = scl;
                return;
            }
        }
    }

    TypeKind Type::GetKind() const noexcept
    {
        return mKind;
    }

    bool Type::Is(TypeCategory category) const noexcept
    {
        return mCategory == category;
    }

    bool Type::IsNumeric() const noexcept
    {
        return IsInteger() || IsFloating();
    }

    bool Type::IsInteger() const noexcept
    {
        return mKind <= TypeKind::U64;
    }

    bool Type::IsFloating() const noexcept
    {
        return mKind == TypeKind::F32 || mKind == TypeKind::F64;
    }

    bool Type::IsBoolean() const noexcept
    {
        return mKind == TypeKind::BOOL;
    }

    bool Type::IsString() const noexcept
    {
        return mKind == TypeKind::STRING;
    }

    bool Type::IsChar() const noexcept
    {
        return mKind == TypeKind::CHAR;
    }

    bool Type::IsAny() const noexcept
    {
        return mKind == TypeKind::ANY;
    }

    STD_STRING_VIEW Type::GetName() const noexcept
    {
        return mName;
    }
}