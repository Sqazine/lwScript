#pragma once
#include "Chunk.h"
#include "Ast.h"
namespace lws
{
    enum class RWState // read write state
    {
        READ,
        WRITE,
    };

    class Compiler
    {
    public:
        Compiler();
        ~Compiler();

        Chunk Compile(Stmt* stmt);

        void ResetStatus();

    private:
        void CompileDeclaration(Stmt *stmt);
        void CompileLetDeclaration(LetStmt *stmt);
        void CompileConstDeclaration(ConstStmt *stmt);
        void CompileFunctionDeclaration(FunctionStmt *stmt);
        void CompileClassDeclaration(ClassStmt *stmt);

        void CompileStmt(Stmt *stmt);
        void CompileExprStmt(ExprStmt *stmt);
        void CompileIfStmt(IfStmt *stmt);
        void CompileScopeStmt(ScopeStmt *stmt);
        void CompileWhileStmt(WhileStmt *stmt);
        void CompileReturnStmt(ReturnStmt *stmt);

        void CompileExpr(Expr *expr, const RWState &state = RWState::READ);
        void CompileInfixExpr(InfixExpr *expr);
        void CompileIntNumExpr(IntNumExpr *expr);
        void CompileRealNumExpr(RealNumExpr *expr);
        void CompileBoolExpr(BoolExpr *expr);
        void CompilePrefixExpr(PrefixExpr *expr);
        void CompilePostfixExpr(PostfixExpr *expr);
        void CompileStrExpr(StrExpr *expr);
        void CompileNullExpr(NullExpr *expr);
        void CompileGroupExpr(GroupExpr *expr);
        void CompileArrayExpr(ArrayExpr *expr);
        void CompileTableExpr(TableExpr *expr);
        void CompileIndexExpr(IndexExpr *expr);
        void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state);
        void CompileLambdaExpr(LambdaExpr *expr);
        void CompileCallExpr(CallExpr *expr);
        void CompileDotExpr(DotExpr *expr, const RWState &state = RWState::READ);
        void CompileRefExpr(RefExpr *expr);

        uint64_t Emit(uint8_t opcode);
        uint64_t EmitUint64(uint64_t opcode);
        uint64_t EmitConstant(const Value& value);
        uint64_t AddConstant(const Value &value);

        Chunk chunk;
    };
}