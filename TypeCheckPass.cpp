#include "TypeCheckPass.h"

namespace lwscript
{
    Stmt *TypeCheckPass::ExecuteAstStmts(AstStmts *stmt)
    {
        for (auto &s : stmt->stmts)
            s = ExecuteStmt(s);

        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteVarStmt(VarStmt *stmt)
    {
        for (auto &[k, v] : stmt->variables)
        {
            if (k->kind == AstKind::ARRAY)
            {
            }
            else if (k->kind == AstKind::VAR_DESC)
            {
                auto leftType = ((VarDescExpr *)k)->type;

                if (leftType.IsAny())
                {
                    return stmt;
                }

                if (v->kind != AstKind::LITERAL)
                    Logger::Error(TEXT("Now Only check Literal Expr type and named variable only."));

                auto rightType = ((LiteralExpr *)v)->type;

                if (leftType.GetKind() == TypeKind::I64)
                {
                    switch (rightType.GetKind())
                    {
                    case TypeKind::I8:
                        Logger::Info(k->tagToken, TEXT("Assignning a int8 value to a int64 value, it is a broadening conversion."));
                        break;
                    case TypeKind::I16:
                        Logger::Info(k->tagToken, TEXT("Assignning a int16 value to a int64 value, it is a broadening conversion."));
                        break;
                    case TypeKind::I32:
                        Logger::Info(k->tagToken, TEXT("Assignning a int32 value to a int64 value, it is a broadening conversion."));
                        break;
                    case TypeKind::U8:
                        Logger::Warn(k->tagToken, TEXT("Assignning a uint8 value to a int64 value, it is a broadening conversion and may lose data."));
                        break;
                    case TypeKind::U16:
                        Logger::Warn(k->tagToken, TEXT("Assignning a uint16 value to a int64 value, it is a broadening conversion and may lose data."));
                        break;
                    case TypeKind::U32:
                        Logger::Warn(k->tagToken, TEXT("Assignning a uint32 value to a int64 value, it is a broadening conversion and may lose data."));
                        break;
                    case TypeKind::U64:
                        Logger::Warn(k->tagToken, TEXT("Assignning a uint64 value to a int64 value, it may lose data."));
                        break;
                    case TypeKind::F32:
                        Logger::Warn(k->tagToken, TEXT("Assignning a floating32 value to a int64 value, it is a broadening conversion and may lose data."));
                        break;
                    case TypeKind::F64:
                        Logger::Warn(k->tagToken, TEXT("Assignning a floating64 value to a int64 value, it may lose data."));
                        break;
                    case TypeKind::I64:
                    default:
                        break;
                    }
                }
            }
        }

        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteExprStmt(ExprStmt *stmt)
    {
        stmt->expr = ExecuteExpr(stmt->expr);
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteReturnStmt(ReturnStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteIfStmt(IfStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteScopeStmt(ScopeStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteWhileStmt(WhileStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteEnumStmt(EnumStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteFunctionStmt(FunctionStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteClassStmt(ClassStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteBreakStmt(BreakStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteContinueStmt(ContinueStmt *stmt)
    {
        return nullptr;
    }
    Stmt *TypeCheckPass::ExecuteModuleStmt(ModuleStmt *stmt)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteLiteralExpr(LiteralExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteInfixExpr(InfixExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecutePrefixExpr(PrefixExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecutePostfixExpr(PostfixExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteConditionExpr(ConditionExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteGroupExpr(GroupExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteArrayExpr(ArrayExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteAppregateExpr(AppregateExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteDictExpr(DictExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteIndexExpr(IndexExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteNewExpr(NewExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteThisExpr(ThisExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteBaseExpr(BaseExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteIdentifierExpr(IdentifierExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteLambdaExpr(LambdaExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteCompoundExpr(CompoundExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteCallExpr(CallExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteDotExpr(DotExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteRefExpr(RefExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteStructExpr(StructExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteVarArgExpr(VarArgExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteFactorialExpr(FactorialExpr *expr)
    {
        return nullptr;
    }
    Expr *TypeCheckPass::ExecuteVarDescExpr(VarDescExpr *expr)
    {
        return nullptr;
    }
}