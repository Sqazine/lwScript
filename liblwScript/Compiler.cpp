#include "Compiler.h"
#include "Utils.h"
#include "Object.h"
namespace lws
{
    Compiler::Compiler()
    {
    }

    Compiler::~Compiler()
    {
    }

    Chunk Compiler::Compile(Stmt *stmt)
    {
        if (stmt->Type() == AST_ASTSTMTS)
        {
            auto stmts = ((AstStmts *)stmt)->stmts;
            for (const auto &s : stmts)
                CompileDeclaration(s);
        }
        else
            CompileDeclaration(stmt);

        //TODO:for debug only
        Emit(OP_RETURN);
        Emit(1);

        return chunk;
    }

    void Compiler::ResetStatus()
    {
    }

    void Compiler::CompileDeclaration(Stmt *stmt)
    {
        switch (stmt->Type())
        {
        case AST_LET:
            CompileLetDeclaration((LetStmt *)stmt);
            break;
        case AST_CONST:
            CompileConstDeclaration((ConstStmt *)stmt);
            break;
        case AST_FUNCTION:
            CompileFunctionDeclaration((FunctionStmt *)stmt);
            break;
        case AST_CLASS:
            CompileClassDeclaration((ClassStmt *)stmt);
            break;
        default:
            CompileStmt((Stmt *)stmt);
            break;
        }
    }
    void Compiler::CompileLetDeclaration(LetStmt *stmt)
    {
    }

    void Compiler::CompileConstDeclaration(ConstStmt *stmt)
    {
    }

    void Compiler::CompileFunctionDeclaration(FunctionStmt *stmt)
    {
    }
    void Compiler::CompileClassDeclaration(ClassStmt *stmt)
    {
    }

    void Compiler::CompileStmt(Stmt *stmt)
    {
        switch (stmt->Type())
        {
        case AST_IF:
            CompileIfStmt((IfStmt *)stmt);
            break;
        case AST_SCOPE:
            CompileScopeStmt((ScopeStmt *)stmt);
            break;
        case AST_WHILE:
            CompileWhileStmt((WhileStmt *)stmt);
            break;
        case AST_RETURN:
            CompileReturnStmt((ReturnStmt *)stmt);
            break;
        default:
            CompileExprStmt((ExprStmt *)stmt);
            break;
        }
    }
    void Compiler::CompileExprStmt(ExprStmt *stmt)
    {
        CompileExpr(stmt->expr);
    }
    void Compiler::CompileIfStmt(IfStmt *stmt)
    {
    }
    void Compiler::CompileScopeStmt(ScopeStmt *stmt)
    {
    }
    void Compiler::CompileWhileStmt(WhileStmt *stmt)
    {
    }
    void Compiler::CompileReturnStmt(ReturnStmt *stmt)
    {
    }

    void Compiler::CompileExpr(Expr *expr, const RWState &state)
    {
        switch (expr->Type())
        {
        case AST_INFIX:
            CompileInfixExpr((InfixExpr *)expr);
            break;
        case AST_INT:
            CompileIntNumExpr((IntNumExpr *)expr);
            break;
        case AST_REAL:
            CompileRealNumExpr((RealNumExpr *)expr);
            break;
        case AST_BOOL:
            CompileBoolExpr((BoolExpr *)expr);
            break;
        case AST_PREFIX:
            CompilePrefixExpr((PrefixExpr *)expr);
            break;
        case AST_POSTFIX:
            CompilePostfixExpr((PostfixExpr *)expr);
            break;
        case AST_STR:
            CompileStrExpr((StrExpr *)expr);
            break;
        case AST_NULL:
            CompileNullExpr((NullExpr *)expr);
            break;
        case AST_GROUP:
            CompileGroupExpr((GroupExpr *)expr);
            break;
        case AST_ARRAY:
            CompileArrayExpr((ArrayExpr *)expr);
            break;
        case AST_TABLE:
            CompileTableExpr((TableExpr *)expr);
            break;
        case AST_INDEX:
            CompileIndexExpr((IndexExpr *)expr);
            break;
        case AST_IDENTIFIER:
            CompileIdentifierExpr((IdentifierExpr *)expr, state);
            break;
        case AST_LAMBDA:
            CompileLambdaExpr((LambdaExpr *)expr);
            break;
        case AST_CALL:
            CompileCallExpr((CallExpr *)expr);
            break;
        case AST_DOT:
            CompileDotExpr((DotExpr *)expr);
            break;
        case AST_REF:
            CompileRefExpr((RefExpr *)expr);
            break;
        default:
            break;
        }
    }
    void Compiler::CompileInfixExpr(InfixExpr *expr)
    {
        CompileExpr(expr->right);
        CompileExpr(expr->left);
        if (expr->op == L"+")
            Emit(OP_ADD);
        else if (expr->op == L"-")
            Emit(OP_SUB);
        else if (expr->op == L"*")
            Emit(OP_MUL);
        else if (expr->op == L"/")
            Emit(OP_DIV);
        else if (expr->op == L"%")
            Emit(OP_MOD);
        else if (expr->op == L"&")
            Emit(OP_BIT_AND);
        else if (expr->op == L"|")
            Emit(OP_BIT_OR);
        else if (expr->op == L"<")
            Emit(OP_LESS);
        else if (expr->op == L">")
            Emit(OP_GREATER);
        else if (expr->op == L"<<")
            Emit(OP_BIT_LEFT_SHIFT);
        else if (expr->op == L">>")
            Emit(OP_BIT_RIGHT_SHIFT);
        else if (expr->op == L"<=")
        {
            Emit(OP_GREATER);
            Emit(OP_NOT);
        }
        else if (expr->op == L">=")
        {
            Emit(OP_LESS);
            Emit(OP_NOT);
        }
        else if (expr->op == L"==")
            Emit(OP_EQUAL);
        else if (expr->op == L"!=")
        {
            Emit(OP_EQUAL);
            Emit(OP_NOT);
        }
    }
    void Compiler::CompileIntNumExpr(IntNumExpr *expr)
    {
        EmitConstant(expr->value);
    }
    void Compiler::CompileRealNumExpr(RealNumExpr *expr)
    {
        EmitConstant(expr->value);
    }
    void Compiler::CompileBoolExpr(BoolExpr *expr)
    {
        EmitConstant(expr->value);
    }
    void Compiler::CompilePrefixExpr(PrefixExpr *expr)
    {
        CompileExpr(expr->right);
        if (expr->op == L"!")
            Emit(OP_NOT);
        else if (expr->op == L"-")
            Emit(OP_MINUS);
        else
            ASSERT(L"No prefix op:" + expr->op);
    }
    void Compiler::CompilePostfixExpr(PostfixExpr *expr)
    {
        CompileExpr(expr->left);
        if (expr->op == L"!")
            Emit(OP_FACTORIAL);
        else
            ASSERT(L"No postfix op:" + expr->op);
    }
    void Compiler::CompileStrExpr(StrExpr *expr)
    {
        EmitConstant(new StrObject(expr->value));
    }
    void Compiler::CompileNullExpr(NullExpr *expr)
    {
    }
    void Compiler::CompileGroupExpr(GroupExpr *expr)
    {
    }
    void Compiler::CompileArrayExpr(ArrayExpr *expr)
    {
    }

    void Compiler::CompileTableExpr(TableExpr *expr)
    {
    }

    void Compiler::CompileIndexExpr(IndexExpr *expr)
    {
    }
    void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state)
    {
    }
    void Compiler::CompileLambdaExpr(LambdaExpr *expr)
    {
    }
    void Compiler::CompileCallExpr(CallExpr *expr)
    {
    }
    void Compiler::CompileDotExpr(DotExpr *expr, const RWState &state)
    {
    }
    void Compiler::CompileRefExpr(RefExpr *expr)
    {
    }

    uint64_t Compiler::Emit(uint8_t opcode)
    {
        chunk.opCodes.emplace_back(opcode);
        return chunk.opCodes.size() - 1;
    }
    uint64_t Compiler::EmitConstant(const Value &value)
    {
        Emit(OP_CONSTANT);
        uint64_t pos = AddConstant(value);
        Emit(uint8_t(pos >> 56));
        Emit(uint8_t(pos >> 48));
        Emit(uint8_t(pos >> 40));
        Emit(uint8_t(pos >> 32));
        Emit(uint8_t(pos >> 24));
        Emit(uint8_t(pos >> 16));
        Emit(uint8_t(pos >> 8));
        Emit(uint8_t(pos >> 0));
        return chunk.opCodes.size() - 1;
    }
    uint64_t Compiler::AddConstant(const Value &value)
    {
        chunk.constants.emplace_back(value);
        return chunk.constants.size() - 1;
    }
}