#include "TypeCheckPass.h"
#include "Logger.h"
namespace lwscript
{
    struct TypeMapInfo
    {
        TypeKind leftKind;
        TypeKind rightKind;
        Logger::Kind logKind;
        STD_STRING msg;
    };

    const TypeMapInfo TypeMaps[] =
        {
            {TypeKind::I8, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int32 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::U8,  Logger::Kind::WARN, TEXT("Assignning a uint8 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a floating32 value to a int8 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I8, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a floating64 value to a int8 value, it is a narrow conversion and may lose data.")},

            {TypeKind::I16, TypeKind::I8,  Logger::Kind::INFO, TEXT("Assignning a int8 value to a int16 value, it is a broadening conversion.")},
            {TypeKind::I16, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int32 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::U8,  Logger::Kind::WARN, TEXT("Assignning a uint8 value to a int16 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I16, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a int16 value, it may lose data.")},
            {TypeKind::I16, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a floating32 value to a int16 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I16, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a floating64 value to a int16 value, it is a narrow conversion and may lose data.")},

            {TypeKind::I32, TypeKind::I8,  Logger::Kind::INFO, TEXT("Assignning a int8 value to a int32 value, it is a broadening conversion.")},
            {TypeKind::I32, TypeKind::I16, Logger::Kind::INFO, TEXT("Assignning a int32 value to a int32 value, it is a broadening conversion.")},
            {TypeKind::I32, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int32 value to a int32 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I32, TypeKind::U8,  Logger::Kind::WARN, TEXT("Assignning a uint8 value to a int32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I32, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a int32 value, it is a broadening conversion and may lose data.")},
            {TypeKind::I32, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a int32 value, it may lose data.")},
            {TypeKind::I32, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a int32 value, it is a narrow conversion and may lose data.")},
            {TypeKind::I32, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a floating32 value to a int32 value, it may lose data.")},
            {TypeKind::I32, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a floating64 value to a int32 value, it is a narrow conversion and may lose data.")},

            {TypeKind::I64, TypeKind::I8,  Logger::Kind::INFO, TEXT("Assignning a int8 value to a int64 value, it is a broadening conversion.")},
            {TypeKind::I64, TypeKind::I16, Logger::Kind::INFO, TEXT("Assignning a int16 value to a int64 value, it is a broadening conversion.")},
            {TypeKind::I64, TypeKind::I32, Logger::Kind::INFO, TEXT("Assignning a int32 value to a int64 value, it is a broadening conversion.")},
            {TypeKind::I64, TypeKind::U8,  Logger::Kind::WARN, TEXT("Assignning a uint8 value to a int64 value, it may lose data.")},
            {TypeKind::I64, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a int64 value, it may lose data.")},
            {TypeKind::I64, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a int64 value, it may lose data.")},
            {TypeKind::I64, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a int64 value, it may lose data.")},
            {TypeKind::I64, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a floating32 value to a int64 value, it may lose data.")},
            {TypeKind::I64, TypeKind::F64, Logger::Kind::WARN, TEXT("Assignning a floating64 value to a int64 value, it may lose data.")},

            {TypeKind::F64, TypeKind::I8,  Logger::Kind::WARN, TEXT("Assignning a int8 value to a floating64 value, it is a broadening conversion and may lose data.")},  
            {TypeKind::F64, TypeKind::I16, Logger::Kind::WARN, TEXT("Assignning a int16 value to a floating64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::I32, Logger::Kind::WARN, TEXT("Assignning a int32 value to a floating64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::I64, Logger::Kind::WARN, TEXT("Assignning a int64 value to a floating64 value, it may lose data.")},
            {TypeKind::F64, TypeKind::U8,  Logger::Kind::WARN, TEXT("Assignning a uint8 value to a floating64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::U16, Logger::Kind::WARN, TEXT("Assignning a uint16 value to a floating64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::U32, Logger::Kind::WARN, TEXT("Assignning a uint32 value to a floating64 value, it is a broadening conversion and may lose data.")},
            {TypeKind::F64, TypeKind::U64, Logger::Kind::WARN, TEXT("Assignning a uint64 value to a floating64 value, it may lose data.")},
            {TypeKind::F64, TypeKind::F32, Logger::Kind::WARN, TEXT("Assignning a floating32 value to a floating64 value, it is a broadening conversion.")},
    };

    bool FindTypeMapInfo(TypeKind left, TypeKind right, STD_STRING &msg)
    {
        for (const auto &t : TypeMaps)
        {
            if (t.leftKind == left && t.rightKind == right)
            {
                msg = t.msg;
                return true;
            }
        }
        return false;
    }

    Stmt *TypeCheckPass::ExecuteAstStmts(AstStmts *stmt)
    {
        for (auto &s : stmt->stmts)
            s = ExecuteStmt(s);

        return stmt;
    }
    Decl *TypeCheckPass::ExecuteVarDecl(VarDecl *decl)
    {
        for (auto &[k, v] : decl->variables)
        {
            if (k->kind == AstKind::ARRAY)
            {
            }
            else if (k->kind == AstKind::VAR_DESC)
            {
                auto leftType = ((VarDescExpr *)k)->type;

                if (leftType.Is(TypeKind::ANY))
                    return decl;

                if (v->kind != AstKind::LITERAL)
                    Logger::Error(TEXT("Now Only check Literal Expr type and named variable only."));

                auto rightType = ((LiteralExpr *)v)->type;

                STD_STRING msg;
                auto flag = FindTypeMapInfo(leftType.GetKind(), rightType.GetKind(), msg);
                if (flag)
                    Logger::Info(k->tagToken, msg);
            }
        }

        return decl;
    }
    Stmt *TypeCheckPass::ExecuteExprStmt(ExprStmt *stmt)
    {
        stmt->expr = ExecuteExpr(stmt->expr);
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteReturnStmt(ReturnStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteIfStmt(IfStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteScopeStmt(ScopeStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteWhileStmt(WhileStmt *stmt)
    {
        return stmt;
    }
    Decl *TypeCheckPass::ExecuteEnumDecl(EnumDecl *decl)
    {
        return decl;
    }
    Decl *TypeCheckPass::ExecuteFunctionDecl(FunctionDecl *decl)
    {
        return decl;
    }
    Decl *TypeCheckPass::ExecuteClassDecl(ClassDecl *decl)
    {
        return decl;
    }
    Stmt *TypeCheckPass::ExecuteBreakStmt(BreakStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteContinueStmt(ContinueStmt *stmt)
    {
        return stmt;
    }
    Decl *TypeCheckPass::ExecuteModuleDecl(ModuleDecl *decl)
    {
        return decl;
    }
    Expr *TypeCheckPass::ExecuteLiteralExpr(LiteralExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteInfixExpr(InfixExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecutePrefixExpr(PrefixExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecutePostfixExpr(PostfixExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteConditionExpr(ConditionExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteGroupExpr(GroupExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteArrayExpr(ArrayExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteAppregateExpr(AppregateExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteDictExpr(DictExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteIndexExpr(IndexExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteNewExpr(NewExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteThisExpr(ThisExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteBaseExpr(BaseExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteIdentifierExpr(IdentifierExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteLambdaExpr(LambdaExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteCompoundExpr(CompoundExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteCallExpr(CallExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteDotExpr(DotExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteRefExpr(RefExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteStructExpr(StructExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteVarArgExpr(VarArgExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteFactorialExpr(FactorialExpr *expr)
    {
        return expr;
    }
    Expr *TypeCheckPass::ExecuteVarDescExpr(VarDescExpr *expr)
    {
        return expr;
    }
}