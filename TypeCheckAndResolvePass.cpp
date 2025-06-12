#include "TypeCheckAndResolvePass.h"
#include "Logger.h"
namespace CynicScript
{
    struct TypeInfo
    {
        TypeInfo() noexcept = default;
        ~TypeInfo() noexcept = default;

        Type type{};
        Permission permission{};
    };

    class TypeInfoTable
    {
    public:
        TypeInfoTable() noexcept = default;
        TypeInfoTable(TypeInfoTable *enclosing) noexcept : mEnclosing(enclosing) {}
        ~TypeInfoTable() noexcept = default;

        bool Find(STRING_VIEW name, TypeInfo &result)
        {
            auto iter = mTypeInfos.find(name);
            if (iter != mTypeInfos.end())
            {
                result = iter->second;
                return true;
            }
            if (mEnclosing)
                return mEnclosing->Find(name, result);
            return false;
        }
        void Define(STRING_VIEW name, const TypeInfo &result)
        {
            mTypeInfos[name] = result;
        }

    private:
        TypeInfoTable *mEnclosing;
        std::unordered_map<STRING_VIEW, TypeInfo> mTypeInfos;
    };

    struct TypeMapInfo
    {
        TypeKind leftKind;
        TypeKind rightKind;
        Logger::Kind logKind;
        const CHAR_T *msg;
    };

    constexpr TypeMapInfo gPrimitiveTypeMaps[] =
        {
            {TypeKind::I8, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::U8, Logger::Kind::WARN, TEXT("Assignning a uint8 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a int8 value, it is a narrow conversion and may lose data.")},

            {TypeKind::I16, TypeKind::I8, Logger::Kind::INFO, TEXT("Assignning a int8 value to a int16 value, it is a broadening conversion.")},
            {TypeKind::I16, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::U8, Logger::Kind::WARN, TEXT("Assignning a uint8 value to a int16 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I16, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a int16 value, it may lose data.")},
            {TypeKind::I16, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a int16 value, it is a narrow conversion and may lose data.")},

            {TypeKind::I32, TypeKind::I8, Logger::Kind::INFO, TEXT("Assignning a int8 value to a int32 value, it is a broadening conversion.")},
            {TypeKind::I32, TypeKind::I16, Logger::Kind::INFO, TEXT("Assignning a int16 value to a int32 value, it is a broadening conversion.")},
            {TypeKind::I32, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a int32 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I32, TypeKind::U8, Logger::Kind::WARN, TEXT("Assignning a uint8 value to a int32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I32, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a int32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I32, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a int32 value, it may lose data.")},
            {TypeKind::I32, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a int32 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I32, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a int32 value, it may lose data.")},
            {TypeKind::I32, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a int32 value, it is a narrow conversion and may lose data.")},

            {TypeKind::I64, TypeKind::I8, Logger::Kind::INFO, TEXT("Assignning a int8 value to a int64 value, it is a broadening conversion.")},
            {TypeKind::I64, TypeKind::I16, Logger::Kind::INFO, TEXT("Assignning a int16 value to a int64 value, it is a broadening conversion.")},
            {TypeKind::I64, TypeKind::I32, Logger::Kind::INFO, TEXT("Assignning a int32 value to a int64 value, it is a broadening conversion.")},
            {TypeKind::I64, TypeKind::U8, Logger::Kind::WARN, TEXT("Assignning a uint8 value to a int64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I64, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a int64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I64, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a int64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I64, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a int64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I64, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a int64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I64, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a int64 value, it is a broadening conversion and may lose data.")},

            {TypeKind::U8, TypeKind::I8, Logger::Kind::WARN, TEXT("Assignning a int8 value to a uint8 value, it may lose data.")},
            {TypeKind::U8, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a uint8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U8, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a uint8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U8, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a uint8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U8, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a uint8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U8, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a uint8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U8, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a uint8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U8, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a uint8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U8, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a uint8 value, it is a narrow conversion and may lose data.")},

            {TypeKind::U16, TypeKind::I8, Logger::Kind::WARN, TEXT("Assignning a int8 value to a uint16 value, it is a broadening conversion and may lose data.")},
            {TypeKind::U16, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a uint16 value, it may lose data.")},
            {TypeKind::U16, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a uint16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U16, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a uint16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U16, TypeKind::U8, Logger::Kind::INFO, TEXT("Assignning a uint8 value to a uint16 value, it is a broadening conversion.")},
            {TypeKind::U16, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a uint16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U16, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a uint16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U16, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a uint16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U16, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a uint16 value, it is a narrow conversion and may lose data.")},

            {TypeKind::U32, TypeKind::I8, Logger::Kind::WARN, TEXT("Assignning a int8 value to a uint32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::U32, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a uint32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::U32, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a uint32 value, it may lose data.")},
            {TypeKind::U32, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a uint32 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U32, TypeKind::U8, Logger::Kind::INFO, TEXT("Assignning a uint8 value to a uint32 value, it is a broadening conversion.")},
            {TypeKind::U32, TypeKind::U16, Logger::Kind::INFO, TEXT("Assignning a uint16 value to a uint32 value, it is a broadening conversion.")},
            {TypeKind::U32, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a uint32 value, it is a narrow conversion and may lose data.")},
            {TypeKind::U32, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a uint32 value, it may lose data.")},
            {TypeKind::U32, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a uint32 value, it is a narrow conversion and may lose data.")},

            {TypeKind::U64, TypeKind::I8, Logger::Kind::WARN, TEXT("Assignning a int8 value to a uint64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::U64, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a uint64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::U64, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a uint64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::U64, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a uint64 value, it may lose data.")},
            {TypeKind::U64, TypeKind::U8, Logger::Kind::INFO, TEXT("Assignning a uint8 value to a uint64 value, it is a broadening conversion.")},
            {TypeKind::U64, TypeKind::U16, Logger::Kind::INFO, TEXT("Assignning a uint16 value to a uint64 value, it is a broadening conversion.")},
            {TypeKind::U64, TypeKind::U32, Logger::Kind::INFO, TEXT("Assignning a uint32 value to a uint64 value, it is a broadening conversion.")},
            {TypeKind::U64, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a uint64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::U64, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a uint64 value, it may lose data.")},

            {TypeKind::F32, TypeKind::I8, Logger::Kind::WARN, TEXT("Assignning a int8 value to a float32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F32, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a float32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F32, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a float32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F32, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a float32 value, it is a narrow conversion and may lose data.")},
            {TypeKind::F32, TypeKind::U8, Logger::Kind::INFO, TEXT("Assignning a uint8 value to a float32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F32, TypeKind::U16, Logger::Kind::INFO, TEXT("Assignning a uint16 value to a float32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F32, TypeKind::U32, Logger::Kind::INFO, TEXT("Assignning a uint32 value to a float32 value, it may lose data")},
            {TypeKind::F32, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a float32 value to a float32 value, it is a narrow conversion and may lose data.")},
            {TypeKind::F32, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a float64 value to a float32 value, it is a narrow conversion and may lose data.")},

            {TypeKind::F64, TypeKind::I8, Logger::Kind::WARN, TEXT("Assignning a int8 value to a float64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a float64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a float64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a float64 value, it may lose data.")},
            {TypeKind::F64, TypeKind::U8, Logger::Kind::WARN, TEXT("Assignning a uint8 value to a float64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a float64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a float64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a float64 value, it may lose data.")},
            {TypeKind::F64, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a float32 value to a float64 value, it is a broadening conversion.")},
    };

    const TypeMapInfo *FindPrimitiveTypeMapInfo(TypeKind left, TypeKind right)
    {
        for (const auto &t : gPrimitiveTypeMaps)
        {
            if (t.leftKind == left && t.rightKind == right)
            {
                return &t;
            }
        }
        return nullptr;
    }

    TypeCheckAndResolvePass::TypeCheckAndResolvePass() noexcept
        : mTypeInfoTable(new TypeInfoTable())
    {
    }

    TypeCheckAndResolvePass::~TypeCheckAndResolvePass() noexcept
    {
        SAFE_DELETE(mTypeInfoTable);
    }

    Stmt *TypeCheckAndResolvePass::ExecuteAstStmts(AstStmts *stmt)
    {
        for (auto &s : stmt->stmts)
            s = ExecuteStmt(s);

        return stmt;
    }
    Decl *TypeCheckAndResolvePass::ExecuteVarDecl(VarDecl *decl)
    {
        for (auto &[k, v] : decl->variables)
        {
            if (k->kind == AstKind::ARRAY)
            {
            }
            else if (k->kind == AstKind::VAR_DESC)
            {
                Type &leftType = ((VarDescExpr *)k)->type;
                Type &rightType = v->type;

                if (leftType.Is(TypeKind::UNDEFINED))
                {
                    leftType = v->type;
                    return decl;
                }
                else if (leftType.Is(TypeKind::ANY))
                {
                    return decl;
                }
                else if (leftType.IsPrimitiveType() && rightType.IsPrimitiveType())
                {
                    if (auto info = FindPrimitiveTypeMapInfo(leftType.GetKind(), rightType.GetKind()))
                    {
                        Logger::Log(info->logKind, k->tagToken, info->msg);
                    }
                }
            }
        }

        return decl;
    }
    Stmt *TypeCheckAndResolvePass::ExecuteExprStmt(ExprStmt *stmt)
    {
        stmt->expr = ExecuteExpr(stmt->expr);
        return stmt;
    }
    Stmt *TypeCheckAndResolvePass::ExecuteReturnStmt(ReturnStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckAndResolvePass::ExecuteIfStmt(IfStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckAndResolvePass::ExecuteScopeStmt(ScopeStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckAndResolvePass::ExecuteWhileStmt(WhileStmt *stmt)
    {
        return stmt;
    }
    Decl *TypeCheckAndResolvePass::ExecuteEnumDecl(EnumDecl *decl)
    {
        return decl;
    }
    Decl *TypeCheckAndResolvePass::ExecuteFunctionDecl(FunctionDecl *decl)
    {
        return decl;
    }
    Decl *TypeCheckAndResolvePass::ExecuteClassDecl(ClassDecl *decl)
    {
        return decl;
    }
    Stmt *TypeCheckAndResolvePass::ExecuteBreakStmt(BreakStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckAndResolvePass::ExecuteContinueStmt(ContinueStmt *stmt)
    {
        return stmt;
    }
    Decl *TypeCheckAndResolvePass::ExecuteModuleDecl(ModuleDecl *decl)
    {
        return decl;
    }
    Expr *TypeCheckAndResolvePass::ExecuteLiteralExpr(LiteralExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteInfixExpr(InfixExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecutePrefixExpr(PrefixExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecutePostfixExpr(PostfixExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteConditionExpr(ConditionExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteGroupExpr(GroupExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteArrayExpr(ArrayExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteAppregateExpr(AppregateExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteDictExpr(DictExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteIndexExpr(IndexExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteNewExpr(NewExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteThisExpr(ThisExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteBaseExpr(BaseExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteIdentifierExpr(IdentifierExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteLambdaExpr(LambdaExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteCompoundExpr(CompoundExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteCallExpr(CallExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteDotExpr(DotExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteRefExpr(RefExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteStructExpr(StructExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteVarArgExpr(VarArgExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteFactorialExpr(FactorialExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckAndResolvePass::ExecuteVarDescExpr(VarDescExpr *expr)
    {
        return expr;
    }
}