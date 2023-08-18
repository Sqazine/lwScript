#pragma once
#include <cstdint>
#include "Ast.h"

namespace lws
{

    class SyntaxChecker
    {
    public:
        SyntaxChecker();
        ~SyntaxChecker();

        Stmt *Check(Stmt *stmt);

    private:
        Stmt *CheckStmt(Stmt *stmt);
        Stmt *CheckAstStmts(AstStmts *stmt);
        Stmt *CheckVarStmt(VarStmt *stmt);
        Stmt *CheckExprStmt(ExprStmt *stmt);
        Stmt *CheckIfStmt(IfStmt *stmt);
        Stmt *CheckScopeStmt(ScopeStmt *stmt);
        Stmt *CheckWhileStmt(WhileStmt *stmt);
        Stmt *CheckEnumStmt(EnumStmt *stmt);
        Stmt *CheckFunctionStmt(FunctionStmt *stmt);
        Stmt *CheckClassStmt(ClassStmt *stmt);
        Stmt *CheckReturnStmt(ReturnStmt *stmt);
        Stmt *CheckBreakStmt(BreakStmt *stmt);
        Stmt *CheckContinueStmt(ContinueStmt *stmt);

        Expr *CheckExpr(Expr *expr);
        Expr *CheckInfixExpr(InfixExpr *expr);
        Expr *CheckIntNumExpr(IntNumExpr *expr);
        Expr *CheckRealNumExpr(RealNumExpr *expr);
        Expr *CheckBoolExpr(BoolExpr *expr);
        Expr *CheckPrefixExpr(PrefixExpr *expr);
        Expr *CheckPostfixExpr(PostfixExpr *expr);
        Expr *CheckConditionExpr(ConditionExpr *expr);
        Expr *CheckStrExpr(StrExpr *expr);
        Expr *CheckNullExpr(NullExpr *expr);
        Expr *CheckGroupExpr(GroupExpr *expr);
        Expr *CheckArrayExpr(ArrayExpr *expr);
        Expr *CheckAppregateExpr(AppregateExpr *expr);
        Expr *CheckDictExpr(DictExpr *expr);
        Expr *CheckIndexExpr(IndexExpr *expr);
        Expr *CheckNewExpr(NewExpr *expr);
        Expr *CheckThisExpr(ThisExpr *expr);
        Expr *CheckBaseExpr(BaseExpr *expr);
        Expr *CheckIdentifierExpr(IdentifierExpr *expr);
        Expr *CheckLambdaExpr(LambdaExpr *expr);
        Expr *CheckBlockExpr(BlockExpr *expr);
        Expr *CheckCallExpr(CallExpr *expr);
        Expr *CheckDotExpr(DotExpr *expr);
        Expr *CheckRefExpr(RefExpr *expr);
        Expr *CheckAnonymousObjExpr(AnonyObjExpr *expr);
        Expr *CheckVarArgExpr(VarArgExpr *expr);
        Expr *CheckFactorialExpr(FactorialExpr *expr);
        Expr *CheckVarDescExpr(VarDescExpr *expr);

        bool IsConstantLiteral(Expr *expr);

        uint32_t mLoopDepth;
        uint32_t mVarArgScopeDepth;
        uint32_t mClassScopeDepth;
    };
}