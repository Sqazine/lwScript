#pragma once
#include "Token.h"
namespace CynicScript
{
    enum class TypeKind
    {
        UNDEFINED = 0,

        // Primitive Type
        I8,
        U8,
        I16,
        U16,
        I32,
        U32,
        I64,
        U64,

        F32,
        F64,

        BOOL,
        CHAR,
        ANY,
        STR,

        // Composite Type
        STRUCT,
        CLASS,
    };

    class Type
    {
    public:
        Type() noexcept;
        Type(TypeKind kind) noexcept;
        Type(STRING_VIEW name) noexcept;
        Type(TypeKind kind, STRING_VIEW name) noexcept;
        ~Type() noexcept = default;

        TypeKind GetKind() const noexcept;

        bool IsNumeric() const noexcept;
        bool IsInteger() const noexcept;
        bool IsFloating() const noexcept;

        bool Is(TypeKind kind)
        {
            return mKind == kind;
        }

        STRING_VIEW GetName() const noexcept;

        bool IsPrimitiveType() const noexcept;
        bool IsCompositeType() const noexcept;

    private:
        void CheckIsValid() const;

        STRING_VIEW mName;
        TypeKind mKind;
    };
};