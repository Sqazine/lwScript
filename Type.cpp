#include "Type.h"

namespace CynicScript
{
    constexpr struct
    {
        STRING_VIEW name;
        TypeKind kind;
    } gPrimitiveTypeMap[] = {
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
        {TEXT("string"), TypeKind::STR},
    };

    Type::Type() noexcept
        : mKind(TypeKind::ANY), mName(TEXT("any"))
    {
    }

    Type::Type(STRING_VIEW name, const SourceLocation &scl) noexcept
    {
        for (const auto &p : gPrimitiveTypeMap)
        {
            if (p.name == name)
            {
                mName = name;
                mKind = p.kind;
                mSourceLocation = scl;
                return;
            }
        }
    }

    TypeKind Type::GetKind() const noexcept
    {
        return mKind;
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

    STRING_VIEW Type::GetName() const noexcept
    {
        return mName;
    }

    bool Type::IsPrimitiveType() const noexcept
    {
        return mKind >= TypeKind::I8 && mKind <= TypeKind::ANY;
    }

    bool Type::IsCompositeType() const noexcept
    {
        return !IsPrimitiveType();
    }
}