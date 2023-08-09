#pragma once
#include <vector>
#include "Ast.h"

namespace lws
{
    class Optimizer
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

        Expr *OptExpr(Expr *expr);
        Expr *OptIntNumExpr(IntNumExpr *expr);
        Expr *OptRealNumExpr(RealNumExpr *expr);
        Expr *OptStrExpr(StrExpr *expr);
        Expr *OptNullExpr(NullExpr *expr);
        Expr *OptBoolExpr(BoolExpr *expr);
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
        Expr* OptFactorialExpr(FactorialExpr* expr);

        Expr* OptFlow(Expr* expr);

        Expr *ConstantFold(Expr *expr);
    };
}