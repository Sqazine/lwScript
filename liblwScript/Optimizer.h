#pragma once
#include <vector>
#include "Ast.h"

namespace lwscript
{
    class LWSCRIPT_API Optimizer
    {
    public:
        Optimizer();
        ~Optimizer();

        Stmt *Opt(Stmt *stmt);

    private:
        Stmt *OptStmt(Stmt *stmt);
        Stmt *OptAstStmts(AstStmts *stmt);
        Stmt *OptVarStmt(VarStmt *stmt);

        Stmt *OptExprStmt(ExprStmt *stmt);
        Stmt *OptReturnStmt(ReturnStmt *stmt);
        Stmt *OptIfStmt(IfStmt *stmt);
        Stmt *OptScopeStmt(ScopeStmt *stmt);
        Stmt *OptWhileStmt(WhileStmt *stmt);
        Stmt *OptEnumStmt(EnumStmt *stmt);
        Stmt *OptFunctionStmt(FunctionStmt *stmt);
        Stmt *OptClassStmt(ClassStmt *stmt);
        Stmt *OptModuleStmt(ModuleStmt *stmt);

        Expr *OptExpr(Expr *expr);
        Expr *OptLiteralExpr(LiteralExpr *expr);
        Expr *OptIdentifierExpr(IdentifierExpr *expr);
        Expr *OptGroupExpr(GroupExpr *expr);
        Expr *OptArrayExpr(ArrayExpr *expr);
        Expr *OptDictExpr(DictExpr *expr);
        Expr *OptPrefixExpr(PrefixExpr *expr);
        Expr *OptInfixExpr(InfixExpr *expr);
        Expr *OptPostfixExpr(PostfixExpr *expr);
        Expr *OptConditionExpr(ConditionExpr *expr);
        Expr *OptIndexExpr(IndexExpr *expr);
        Expr *OptRefExpr(RefExpr *expr);
        Expr *OptLambdaExpr(LambdaExpr *expr);
        Expr *OptDotExpr(DotExpr *expr);
        Expr *OptCallExpr(CallExpr *expr);
        Expr *OptNewExpr(NewExpr *expr);
        Expr *OptThisExpr(ThisExpr *expr);
        Expr *OptBaseExpr(BaseExpr *expr);
        Expr *OptFactorialExpr(FactorialExpr *expr);
        Expr *OptVarDescExpr(VarDescExpr *expr);

        Expr *OptFlow(Expr *expr);

        Expr *ConstantFold(Expr *expr);
    };
}