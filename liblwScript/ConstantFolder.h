#pragma once
#include <vector>
#include "Ast.h"

namespace lwscript
{
    class LWSCRIPT_API ConstantFolder
    {
    public:
        ConstantFolder() =default;
        ~ConstantFolder()=default;

        Stmt *Fold(Stmt *stmt);

    private:
        Stmt *FoldStmt(Stmt *stmt);
        Stmt *FoldAstStmts(AstStmts *stmt);
        Stmt *FoldVarStmt(VarStmt *stmt);

        Stmt *FoldExprStmt(ExprStmt *stmt);
        Stmt *FoldReturnStmt(ReturnStmt *stmt);
        Stmt *FoldIfStmt(IfStmt *stmt);
        Stmt *FoldScopeStmt(ScopeStmt *stmt);
        Stmt *FoldWhileStmt(WhileStmt *stmt);
        Stmt *FoldEnumStmt(EnumStmt *stmt);
        Stmt *FoldFunctionStmt(FunctionStmt *stmt);
        Stmt *FoldClassStmt(ClassStmt *stmt);
        Stmt *FoldModuleStmt(ModuleStmt *stmt);

        Expr *FoldExpr(Expr *expr);
        Expr *FoldLiteralExpr(LiteralExpr *expr);
        Expr *FoldIdentifierExpr(IdentifierExpr *expr);
        Expr *FoldGroupExpr(GroupExpr *expr);
        Expr *FoldArrayExpr(ArrayExpr *expr);
        Expr *FoldDictExpr(DictExpr *expr);
        Expr *FoldPrefixExpr(PrefixExpr *expr);
        Expr *FoldInfixExpr(InfixExpr *expr);
        Expr *FoldPostfixExpr(PostfixExpr *expr);
        Expr *FoldConditionExpr(ConditionExpr *expr);
        Expr *FoldIndexExpr(IndexExpr *expr);
        Expr *FoldRefExpr(RefExpr *expr);
        Expr *FoldLambdaExpr(LambdaExpr *expr);
        Expr *FoldDotExpr(DotExpr *expr);
        Expr *FoldCallExpr(CallExpr *expr);
        Expr *FoldNewExpr(NewExpr *expr);
        Expr *FoldThisExpr(ThisExpr *expr);
        Expr *FoldBaseExpr(BaseExpr *expr);
        Expr *FoldFactorialExpr(FactorialExpr *expr);
        Expr *FoldVarDescExpr(VarDescExpr *expr);

        Expr *ConstantFold(Expr *expr);
    };
}