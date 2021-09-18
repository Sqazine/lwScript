#pragma once
#include <vector>
#include <string>
#include "Ast.h"
#include "Frame.h"
namespace lwScript
{
    enum State
    {
        INIT,
        READ,
        WRITE
    };
    class Compiler
    {
    public:
        Compiler();
        ~Compiler();

        const Frame& Compile(Stmt* stmt);

        void ResetStatus();
    private:
        void CompileAstStmts(AstStmts* stmt,Frame& frame);
        void CompileStmt(Stmt* stmt,Frame& frame);
        void CompileReturnStmt(ReturnStmt* stmt,Frame& frame);
        void CompileExprStmt(ExprStmt* stmt,Frame& frame);
        void CompileLetStmt(LetStmt* stmt,Frame& frame);
        void CompileScopeStmt(ScopeStmt* stmt,Frame& frame);

        void CompileExpr(Expr* expr,Frame& frame, State state = READ);
        void CompileNumExpr(NumExpr* expr,Frame& frame);
        void CompileStrExpr(StrExpr* expr,Frame& frame);
        void CompileBoolExpr(BoolExpr* expr,Frame& frame);
        void CompileNilExpr(NilExpr* expr,Frame& frame);
        void CompileIdentifierExpr(IdentifierExpr* expr,Frame& frame, State state=READ);
        void CompileGroupExpr(GroupExpr* expr,Frame& frame);
        void CompileArrayExpr(ArrayExpr* expr,Frame& frame);
        void CompileIndexExpr(IndexExpr* expr,Frame& frame, State state=READ);
        void CompileFunctionExpr(FunctionExpr* expr,Frame& frame);
        void CompileStructExpr(StructExpr* expr,Frame& frame);
        void CompilePrefixExpr(PrefixExpr* expr,Frame& frame);
        void CompileInfixExpr(InfixExpr* expr,Frame& frame);

        Frame m_RootFrame;
    };
}