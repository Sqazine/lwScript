#pragma once
#include <vector>
#include "Ast.h"
#include "AstPass.h "

namespace lwScript
{
#ifdef LWS_CONSTANT_FOLD_OPT
    class LWS_API ConstantFoldPass : public AstPass
    {
    public:
        ConstantFoldPass() noexcept = default;
        virtual ~ConstantFoldPass() override = default;

    protected:
        Stmt *ExecuteAstStmts(AstStmts *stmt) override;

        Decl *ExecuteVarDecl(VarDecl * decl) override;
        Decl *ExecuteFunctionDecl(FunctionDecl * decl) override;
        Decl *ExecuteModuleDecl(ModuleDecl * decl) override;
        Decl *ExecuteEnumDecl(EnumDecl * decl) override;
        Decl *ExecuteClassDecl(ClassDecl * decl) override;
    
        Stmt *ExecuteExprStmt(ExprStmt *stmt) override;
        Stmt *ExecuteReturnStmt(ReturnStmt *stmt) override;
        Stmt *ExecuteIfStmt(IfStmt *stmt) override;
        Stmt *ExecuteScopeStmt(ScopeStmt *stmt) override;
        Stmt *ExecuteWhileStmt(WhileStmt *stmt) override;
        Stmt *ExecuteBreakStmt(BreakStmt *stmt) override;
        Stmt *ExecuteContinueStmt(ContinueStmt *stmt) override;

        Expr *ExecuteLiteralExpr(LiteralExpr *expr) override;
        Expr *ExecuteIdentifierExpr(IdentifierExpr *expr) override;
        Expr *ExecuteGroupExpr(GroupExpr *expr) override;
        Expr *ExecuteArrayExpr(ArrayExpr *expr) override;
        Expr *ExecuteAppregateExpr(AppregateExpr *expr) override;
        Expr *ExecuteDictExpr(DictExpr *expr) override;
        Expr *ExecutePrefixExpr(PrefixExpr *expr) override;
        Expr *ExecuteInfixExpr(InfixExpr *expr) override;
        Expr *ExecutePostfixExpr(PostfixExpr *expr) override;
        Expr *ExecuteConditionExpr(ConditionExpr *expr) override;
        Expr *ExecuteIndexExpr(IndexExpr *expr) override;
        Expr *ExecuteRefExpr(RefExpr *expr) override;
        Expr *ExecuteStructExpr(StructExpr *expr) override;
        Expr *ExecuteVarArgExpr(VarArgExpr *expr) override;
        Expr *ExecuteLambdaExpr(LambdaExpr *expr) override;
        Expr *ExecuteCompoundExpr(CompoundExpr *expr) override;
        Expr *ExecuteDotExpr(DotExpr *expr) override;
        Expr *ExecuteCallExpr(CallExpr *expr) override;
        Expr *ExecuteNewExpr(NewExpr *expr) override;
        Expr *ExecuteThisExpr(ThisExpr *expr) override;
        Expr *ExecuteBaseExpr(BaseExpr *expr) override;
        Expr *ExecuteFactorialExpr(FactorialExpr *expr) override;
        Expr *ExecuteVarDescExpr(VarDescExpr *expr) override;

        Expr *ConstantFold(Expr *expr);
    };
#endif
}