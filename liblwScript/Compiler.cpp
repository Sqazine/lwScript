#include "Compiler.h"
#include "Utils.h"
#include "Object.h"
namespace lws
{
    Compiler::Compiler()
        : mSymbolTable(nullptr)
    {
        ResetStatus();
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

        return CurChunk();
    }

    void Compiler::ResetStatus()
    {
        std::vector<Chunk>().swap(mChunkList);
        mChunkList.emplace_back(Chunk());
        if (mSymbolTable)
            delete mSymbolTable;
        mSymbolTable = new SymbolTable();
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
        for (const auto &[k, v] : stmt->variables)
        {
            CompileExpr(v.value);
            auto symbol = mSymbolTable->Define(SymbolDescType::VARIABLE, k->literal);
            if (symbol.type == SymbolType::GLOBAL)
            {
                Emit(OP_SET_GLOBAL);
                Emit(symbol.idx);
            }
        }
    }

    void Compiler::CompileConstDeclaration(ConstStmt *stmt)
    {
        for (const auto &[k, v] : stmt->consts)
        {
            CompileExpr(v.value);
            auto symbol = mSymbolTable->Define(SymbolDescType::CONSTANT, k->literal);
            if (symbol.type == SymbolType::GLOBAL)
            {
                Emit(OP_SET_GLOBAL);
                Emit(symbol.idx);
            }
        }
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
        CompileExpr(stmt->expr);
        Emit(OP_RETURN);
        Emit(1);
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
        if(expr->op==L"=")
        {
            CompileExpr(expr->right);
            CompileExpr(expr->left,RWState::WRITE);
        }
        else if (expr->op == L"&&")
        {
            //Short circuit calculation
            CompileExpr(expr->left);
            uint64_t address = EmitJump(OP_JUMP_IF_FALSE);
            Emit(OP_POP);
            CompileExpr(expr->right);
            PatchJump(address);
        }
        else if (expr->op == L"||")
        {
            CompileExpr(expr->left);
            uint64_t elseJumpAddress = EmitJump(OP_JUMP_IF_FALSE);
            uint64_t jumpAddress = EmitJump(OP_JUMP);
            PatchJump(elseJumpAddress);
            Emit(OP_POP);
            CompileExpr(expr->right);
            PatchJump(jumpAddress);
        }
        else
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
        EmitConstant(Value());
    }
    void Compiler::CompileGroupExpr(GroupExpr *expr)
    {
        CompileExpr(expr->expr);
    }
    void Compiler::CompileArrayExpr(ArrayExpr *expr)
    {
        for (const auto &e : expr->elements)
            CompileExpr(e);
        Emit(OP_ARRAY);

        uint64_t pos = expr->elements.size();
        EmitUint64(pos);
    }

    void Compiler::CompileTableExpr(TableExpr *expr)
    {
        for (const auto &[k, v] : expr->elements)
        {
            CompileExpr(v);
            CompileExpr(k);
        }
        Emit(OP_TABLE);
        uint64_t pos = expr->elements.size();
        EmitUint64(pos);
    }

    void Compiler::CompileIndexExpr(IndexExpr *expr)
    {
        CompileExpr(expr->ds);
        CompileExpr(expr->index);
        Emit(OP_INDEX);
    }
    void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state)
    {
        OpCode getOp, setOp;
        auto symbol = mSymbolTable->Resolve(expr->literal);
        if (symbol.type == SymbolType::GLOBAL)
        {
            getOp = OP_GET_GLOBAL;
            setOp = OP_SET_GLOBAL;
        }

        if (state == RWState::WRITE)
        {
            if (symbol.descType == SymbolDescType::VARIABLE)
            {
                Emit(setOp);
                Emit(symbol.idx);
            }
            else
                ASSERT("Constant cannot be assigned!");
        }
        else
        {
            Emit(getOp);
            Emit(symbol.idx);
        }
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
        CurOpCodes().emplace_back(opcode);
        return CurOpCodes().size() - 1;
    }

    uint64_t Compiler::EmitUint64(uint64_t opcode)
    {
        auto decoded = DecodeUint64(opcode);
        Emit(decoded[0]);
        Emit(decoded[1]);
        Emit(decoded[2]);
        Emit(decoded[3]);
        Emit(decoded[4]);
        Emit(decoded[5]);
        Emit(decoded[6]);
        return Emit(decoded[7]);
    }

    uint64_t Compiler::EmitConstant(const Value &value)
    {
        Emit(OP_CONSTANT);
        uint64_t pos = AddConstant(value);
        EmitUint64(pos);
        return CurOpCodes().size() - 1;
    }

    uint64_t Compiler::EmitJump(uint8_t opcode)
    {
        Emit(opcode);
        EmitUint64(0xFFFFFFFFFFFFFFFF);
        return CurOpCodes().size() - 8;
    }

    void Compiler::PatchJump(uint64_t offset)
    {
        uint64_t jumpAddress = CurOpCodes().size() - 1;
        CurOpCodes()[offset + 0] = (jumpAddress >> 56) & 0xFF;
        CurOpCodes()[offset + 1] = (jumpAddress >> 48) & 0xFF;
        CurOpCodes()[offset + 2] = (jumpAddress >> 40) & 0xFF;
        CurOpCodes()[offset + 3] = (jumpAddress >> 32) & 0xFF;
        CurOpCodes()[offset + 4] = (jumpAddress >> 24) & 0xFF;
        CurOpCodes()[offset + 5] = (jumpAddress >> 16) & 0xFF;
        CurOpCodes()[offset + 6] = (jumpAddress >> 8) & 0xFF;
        CurOpCodes()[offset + 7] = (jumpAddress >> 0) & 0xFF;
    }

    uint64_t Compiler::AddConstant(const Value &value)
    {
        CurChunk().constants.emplace_back(value);
        return CurChunk().constants.size() - 1;
    }

    Chunk &Compiler::CurChunk()
    {
        return mChunkList.back();
    }
    OpCodes &Compiler::CurOpCodes()
    {
        return CurChunk().opCodes;
    }
}