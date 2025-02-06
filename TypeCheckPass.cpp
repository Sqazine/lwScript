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

                if (leftType.Is(TypeKind::ANY))
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
    Stmt *TypeCheckPass::ExecuteEnumStmt(EnumStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteFunctionStmt(FunctionStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteClassStmt(ClassStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteBreakStmt(BreakStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteContinueStmt(ContinueStmt *stmt)
    {
        return stmt;
    }
    Stmt *TypeCheckPass::ExecuteModuleStmt(ModuleStmt *stmt)
    {
        return stmt;
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