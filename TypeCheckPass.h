#pragma once
#include "AstPass.h"
namespace lwScript
{
    class TypeInfoTable;
    class LWS_API TypeCheckPass : public AstPass
    {
    public:
        TypeCheckPass() noexcept;
        virtual ~TypeCheckPass() override;

    protected:
        virtual Stmt *ExecuteAstStmts(AstStmts *stmt) override;

        virtual Decl *ExecuteVarDecl(VarDecl *decl) override;
        virtual Decl *ExecuteFunctionDecl(FunctionDecl *decl) override;
        virtual Decl *ExecuteModuleDecl(ModuleDecl *decl) override;
        virtual Decl *ExecuteEnumDecl(EnumDecl *decl) override;
        virtual Decl *ExecuteClassDecl(ClassDecl *decl) override;

        virtual Stmt *ExecuteExprStmt(ExprStmt *stmt) override;
        virtual Stmt *ExecuteReturnStmt(ReturnStmt *stmt) override;
        virtual Stmt *ExecuteIfStmt(IfStmt *stmt) override;
        virtual Stmt *ExecuteScopeStmt(ScopeStmt *stmt) override;
        virtual Stmt *ExecuteWhileStmt(WhileStmt *stmt) override;
        virtual Stmt *ExecuteBreakStmt(BreakStmt *stmt) override;
        virtual Stmt *ExecuteContinueStmt(ContinueStmt *stmt) override;

        virtual Expr *ExecuteLiteralExpr(LiteralExpr *expr) override;
        virtual Expr *ExecuteInfixExpr(InfixExpr *expr) override;
        virtual Expr *ExecutePrefixExpr(PrefixExpr *expr) override;
        virtual Expr *ExecutePostfixExpr(PostfixExpr *expr) override;
        virtual Expr *ExecuteConditionExpr(ConditionExpr *expr) override;
        virtual Expr *ExecuteGroupExpr(GroupExpr *expr) override;
        virtual Expr *ExecuteArrayExpr(ArrayExpr *expr) override;
        virtual Expr *ExecuteAppregateExpr(AppregateExpr *expr) override;
        virtual Expr *ExecuteDictExpr(DictExpr *expr) override;
        virtual Expr *ExecuteIndexExpr(IndexExpr *expr) override;
        virtual Expr *ExecuteNewExpr(NewExpr *expr) override;
        virtual Expr *ExecuteThisExpr(ThisExpr *expr) override;
        virtual Expr *ExecuteBaseExpr(BaseExpr *expr) override;
        virtual Expr *ExecuteIdentifierExpr(IdentifierExpr *expr) override;
        virtual Expr *ExecuteLambdaExpr(LambdaExpr *expr) override;
        virtual Expr *ExecuteCompoundExpr(CompoundExpr *expr) override;
        virtual Expr *ExecuteCallExpr(CallExpr *expr) override;
        virtual Expr *ExecuteDotExpr(DotExpr *expr) override;
        virtual Expr *ExecuteRefExpr(RefExpr *expr) override;
        virtual Expr *ExecuteStructExpr(StructExpr *expr) override;
        virtual Expr *ExecuteVarArgExpr(VarArgExpr *expr) override;
        virtual Expr *ExecuteFactorialExpr(FactorialExpr *expr) override;
        virtual Expr *ExecuteVarDescExpr(VarDescExpr *expr) override;

    private:
        TypeInfoTable* mTypeInfoTable{nullptr};
    };
}