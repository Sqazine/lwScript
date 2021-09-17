#pragma once
#include <vector>
#include <string>
#include "Ast.h"
#include "Chunk.h"
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

        const Chunk& Compile(Stmt* stmt);

        void ResetStatus();
    private:
        void CompileAstStmts(AstStmts* stmt);
        void CompileStmt(Stmt* stmt);
        void CompileReturnStmt(ReturnStmt* stmt);
        void CompileExprStmt(ExprStmt* stmt);
        void CompileLetStmt(LetStmt* stmt);
        void CompileScopeStmt(ScopeStmt* stmt);

        void CompileExpr(Expr* expr, State state = READ);
        void CompileNumExpr(NumExpr* expr);
        void CompileStrExpr(StrExpr* expr);
        void CompileBoolExpr(BoolExpr* expr);
        void CompileNilExpr(NilExpr* expr);
        void CompileIdentifierExpr(IdentifierExpr* expr, State state=READ);
        void CompileGroupExpr(GroupExpr* expr);
        void CompileArrayExpr(ArrayExpr* expr);
        void CompileIndexExpr(IndexExpr* expr, State state=READ);
        void CompilePrefixExpr(PrefixExpr* expr);
        void CompileInfixExpr(InfixExpr* expr);

        Chunk m_Chunk;
    };
}