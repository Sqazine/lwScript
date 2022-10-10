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

    FunctionObject *Compiler::Compile(Stmt *stmt)
    {
        if (stmt->Type() == AST_ASTSTMTS)
        {
            auto stmts = ((AstStmts *)stmt)->stmts;
            for (const auto &s : stmts)
                CompileDeclaration(s);
        }
        else
            CompileDeclaration(stmt);

        Emit(OP_RETURN);
        Emit(0);

        return CurFunction();
    }

    void Compiler::ResetStatus()
    {
        std::vector<FunctionObject *>().swap(mFunctionList);
        mFunctionList.emplace_back(new FunctionObject(L"main"));

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
        auto postfixExprs = StatsPostfixExprs(stmt);

        for (const auto &[k, v] : stmt->variables)
        {
            auto idx = mSymbolTable->Declare(SymbolDescType::VARIABLE, k->literal);
            CompileExpr(v.value);
            auto symbol = mSymbolTable->Define(idx);
            if (symbol.type == SymbolType::GLOBAL)
            {
                Emit(OP_SET_GLOBAL);
                Emit(symbol.idx);
                Emit(OP_POP);
            }
            else if (symbol.type == SymbolType::LOCAL)
            {
                Emit(OP_SET_LOCAL);
                Emit(symbol.idx);
            }
        }

        if (!postfixExprs.empty())
        {
            for (const auto &postfixExpr : postfixExprs)
                CompilePostfixExpr((PostfixExpr *)postfixExpr, false);
        }
    }

    void Compiler::CompileConstDeclaration(ConstStmt *stmt)
    {
        auto postfixExprs = StatsPostfixExprs(stmt);

        for (const auto &[k, v] : stmt->consts)
        {
            auto idx = mSymbolTable->Declare(SymbolDescType::CONSTANT, k->literal);
            CompileExpr(v.value);
            auto symbol = mSymbolTable->Define(idx);
            if (symbol.type == SymbolType::GLOBAL)
            {
                Emit(OP_SET_GLOBAL);
                Emit(symbol.idx);
            }
            else if (symbol.type == SymbolType::LOCAL)
            {
                Emit(OP_SET_LOCAL);
                Emit(symbol.idx);
            }
        }

        if (!postfixExprs.empty())
        {
            for (const auto &postfixExpr : postfixExprs)
                CompilePostfixExpr((PostfixExpr *)postfixExpr, false);
        }
    }

    void Compiler::CompileFunctionDeclaration(FunctionStmt *stmt)
    {
        auto idx = mSymbolTable->Declare(SymbolDescType::CONSTANT, stmt->name->literal);

        mFunctionList.emplace_back(new FunctionObject(stmt->name->literal));
        mSymbolTable = new SymbolTable(mSymbolTable);

        EnterScope();

        CurFunction()->arity = stmt->parameters.size();
        for (const auto &param : stmt->parameters)
        {
            auto idx = mSymbolTable->Declare(SymbolDescType::VARIABLE, param->literal);
            auto symbol = mSymbolTable->Define(idx);
        }

        CompileScopeStmt(stmt->body);

        Emit(OP_RETURN);
        Emit(0);

        ExitScope();
        mSymbolTable = mSymbolTable->enclosing;

        auto function = mFunctionList.back();
        mFunctionList.pop_back();

        EmitConstant(function);

        auto symbol = mSymbolTable->Define(idx);
        if (symbol.type == SymbolType::GLOBAL)
        {
            Emit(OP_SET_GLOBAL);
            Emit(symbol.idx);
            Emit(OP_POP);
        }
        else if (symbol.type == SymbolType::LOCAL)
        {
            Emit(OP_SET_LOCAL);
            Emit(symbol.idx);
        }
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
        {
            EnterScope();
            CompileScopeStmt((ScopeStmt *)stmt);
            ExitScope();
            break;
        }
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
        auto postfixExprs = StatsPostfixExprs(stmt->expr);

        CompileExpr(stmt->expr);
        Emit(OP_POP);

        if (!postfixExprs.empty())
        {
            for (const auto &postfixExpr : postfixExprs)
                CompilePostfixExpr((PostfixExpr *)postfixExpr, false);
        }
    }
    void Compiler::CompileIfStmt(IfStmt *stmt)
    {
        auto conditionPostfixExprs = StatsPostfixExprs(stmt->condition);

        CompileExpr(stmt->condition);

        if (!conditionPostfixExprs.empty())
        {
            for (const auto &postfixExpr : conditionPostfixExprs)
                CompilePostfixExpr((PostfixExpr *)postfixExpr, false);
        }

        auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE);

        CompileStmt(stmt->thenBranch);

        auto jmpAddress = EmitJump(OP_JUMP);

        PatchJump(jmpIfFalseAddress);

        if (stmt->elseBranch)
            CompileStmt(stmt->elseBranch);

        PatchJump(jmpAddress);
    }
    void Compiler::CompileScopeStmt(ScopeStmt *stmt)
    {
        for (const auto &s : stmt->stmts)
            CompileDeclaration(s);
    }
    void Compiler::CompileWhileStmt(WhileStmt *stmt)
    {
        uint64_t jmpAddress = CurOpCodes().size() - 1;

        auto conditionPostfixExprs = StatsPostfixExprs(stmt->condition);

        CompileExpr(stmt->condition);

        if (!conditionPostfixExprs.empty())
        {
            for (const auto &postfixExpr : conditionPostfixExprs)
                CompilePostfixExpr((PostfixExpr *)postfixExpr, false);
        }

        auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE);
        CompileStmt(stmt->body);
        if (stmt->increment)
            CompileStmt(stmt->increment);
        Emit(OP_JUMP);
        EmitUint64(jmpAddress);

        PatchJump(jmpIfFalseAddress);
    }
    void Compiler::CompileReturnStmt(ReturnStmt *stmt)
    {
        auto postfixExprs = StatsPostfixExprs(stmt->expr);

        if (stmt->expr)
        {
            CompileExpr(stmt->expr);
            Emit(OP_RETURN);
            Emit(1);
        }
        else
        {
            Emit(OP_RETURN);
            Emit(0);
        }

        if (!postfixExprs.empty())
        {
            for (const auto &postfixExpr : postfixExprs)
                CompilePostfixExpr((PostfixExpr *)postfixExpr, false);
        }
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
        if (expr->op == L"=")
        {
            CompileExpr(expr->right);
            CompileExpr(expr->left, RWState::WRITE);
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
            CompileExpr(expr->left);
            CompileExpr(expr->right);
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
        else if (expr->op == L"++")
        {
            while (expr->right->Type() == AST_PREFIX && ((PrefixExpr *)expr->right)->op == L"++" || ((PrefixExpr *)expr->right)->op == L"--")
                expr = (PrefixExpr *)expr->right;
            EmitConstant((int64_t)1);
            Emit(OP_ADD);
            CompileExpr(expr->right, RWState::WRITE);
            CompileExpr(expr->right, RWState::READ);
        }
        else if (expr->op == L"--")
        {
            while (expr->right->Type() == AST_PREFIX && ((PrefixExpr *)expr->right)->op == L"++" || ((PrefixExpr *)expr->right)->op == L"--")
                expr = (PrefixExpr *)expr->right;
            EmitConstant((int64_t)1);
            Emit(OP_SUB);
            CompileExpr(expr->right, RWState::WRITE);
            CompileExpr(expr->right, RWState::READ);
        }
        else
            ASSERT(L"No prefix op:" + expr->op);
    }
    void Compiler::CompilePostfixExpr(PostfixExpr *expr, bool isDelayCompile)
    {
        CompileExpr(expr->left);
        if (expr->op == L"!")
            Emit(OP_FACTORIAL);
        else if (!isDelayCompile)
        {
            EmitConstant((int64_t)1);
            if (expr->op == L"++")
                Emit(OP_ADD);
            else if (expr->op == L"--")
                Emit(OP_SUB);
            else
                ASSERT(L"No postfix op:" + expr->op);
            CompileExpr(expr->left, RWState::WRITE);
        }
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
        else if (symbol.type == SymbolType::LOCAL)
        {
            getOp = OP_GET_LOCAL;
            setOp = OP_SET_LOCAL;
        }

        if (state == RWState::WRITE)
        {
            if (symbol.descType == SymbolDescType::VARIABLE)
            {
                Emit(setOp);
                Emit(symbol.idx);

                if (symbol.type == SymbolType::GLOBAL)
                    Emit(OP_POP);
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

    void Compiler::EnterScope()
    {
        mSymbolTable->mScopeDepth++;
    }
    void Compiler::ExitScope()
    {
        mSymbolTable->mScopeDepth--;

        for (int32_t i = 0; i < mSymbolTable->mSymbols.size(); ++i)
        {
            if (mSymbolTable->mSymbols[i].type == SymbolType::LOCAL &&
                mSymbolTable->mSymbols[i].depth > mSymbolTable->mScopeDepth)
            {
                Emit(OP_POP);
                mSymbolTable->mSymbols[i].type = SymbolType::GLOBAL; //mark as global to avoid second pop
            }
        }
    }

    Chunk &Compiler::CurChunk()
    {
        return CurFunction()->chunk;
    }

    FunctionObject *Compiler::CurFunction()
    {
        return mFunctionList.back();
    }

    OpCodes &Compiler::CurOpCodes()
    {
        return CurChunk().opCodes;
    }

    std::vector<Expr *> Compiler::StatsPostfixExprs(AstNode *astNode)
    {
        if (!astNode) //check astnode is nullptr
            return {};

        switch (astNode->Type())
        {
        case AST_BREAK:
        case AST_CONTINUE:
        case AST_REAL:
        case AST_INT:
        case AST_STR:
        case AST_BOOL:
        case AST_NULL:
        case AST_IDENTIFIER:
            return {};
        case AST_ASTSTMTS:
        {
            std::vector<Expr *> result;
            for (const auto &stmt : ((AstStmts *)astNode)->stmts)
            {
                auto stmtResult = StatsPostfixExprs(stmt);
                result.insert(result.end(), stmtResult.begin(), stmtResult.end());
            }
            return result;
        }
        case AST_RETURN:
            return StatsPostfixExprs(((ReturnStmt *)astNode)->expr);
        case AST_EXPR:
            return StatsPostfixExprs(((ExprStmt *)astNode)->expr);
        case AST_LET:
        {
            std::vector<Expr *> result;
            for (const auto &[k, v] : ((LetStmt *)astNode)->variables)
            {
                auto varResult = StatsPostfixExprs(v.value);
                result.insert(result.end(), varResult.begin(), varResult.end());
            }
            return result;
        }
        case AST_CONST:
        {
            std::vector<Expr *> result;
            for (const auto &[k, v] : ((ConstStmt *)astNode)->consts)
            {
                auto varResult = StatsPostfixExprs(v.value);
                result.insert(result.end(), varResult.begin(), varResult.end());
            }
            return result;
        }
        case AST_SCOPE:
        {
            std::vector<Expr *> result;
            for (const auto &stmt : ((ScopeStmt *)astNode)->stmts)
            {
                auto stmtResult = StatsPostfixExprs(stmt);
                result.insert(result.end(), stmtResult.begin(), stmtResult.end());
            }
            return result;
        }
        case AST_IF:
        {
            std::vector<Expr *> result;
            auto conditionResult = StatsPostfixExprs(((IfStmt *)astNode)->condition);
            result.insert(result.end(), conditionResult.begin(), conditionResult.end());
            auto thenBranchResult = StatsPostfixExprs(((IfStmt *)astNode)->thenBranch);
            result.insert(result.end(), thenBranchResult.begin(), thenBranchResult.end());
            auto elseBranchResult = StatsPostfixExprs(((IfStmt *)astNode)->elseBranch);
            result.insert(result.end(), elseBranchResult.begin(), elseBranchResult.end());
            return result;
        }
        case AST_WHILE:
        {
            std::vector<Expr *> result = StatsPostfixExprs(((WhileStmt *)astNode)->condition);
            auto bodyResult = StatsPostfixExprs(((WhileStmt *)astNode)->body);
            result.insert(result.end(), bodyResult.begin(), bodyResult.end());
            if (((WhileStmt *)astNode)->increment)
            {
                auto incrementResult = StatsPostfixExprs(((WhileStmt *)astNode)->increment);
                result.insert(result.end(), incrementResult.begin(), incrementResult.end());
            }
            return result;
        }
        case AST_ENUM:
        {
            std::vector<Expr *> result;
            for (const auto &[k, v] : ((EnumStmt *)astNode)->enumItems)
            {
                auto kResult = StatsPostfixExprs(k);
                auto vResult = StatsPostfixExprs(v);
                result.insert(result.end(), kResult.begin(), kResult.end());
                result.insert(result.end(), vResult.begin(), vResult.end());
            }
            return result;
        }
        case AST_FUNCTION:
        {
            std::vector<Expr *> result;
            auto bodyResult = StatsPostfixExprs(((FunctionStmt *)astNode)->body);
            result.insert(result.end(), bodyResult.begin(), bodyResult.end());
            return result;
        }
        case AST_CLASS:
        {
            std::vector<Expr *> result;

            for (const auto &letStmt : ((ClassStmt *)astNode)->letStmts)
            {
                auto letStmtResult = StatsPostfixExprs(letStmt);
                result.insert(result.end(), letStmtResult.begin(), letStmtResult.end());
            }

            for (const auto &constStmt : ((ClassStmt *)astNode)->constStmts)
            {
                auto constStmtResult = StatsPostfixExprs(constStmt);
                result.insert(result.end(), constStmtResult.begin(), constStmtResult.end());
            }

            for (const auto &fnStmt : ((ClassStmt *)astNode)->fnStmts)
            {
                auto fnStmtResult = StatsPostfixExprs(fnStmt);
                result.insert(result.end(), fnStmtResult.begin(), fnStmtResult.end());
            }
            return result;
        }
        case AST_GROUP:
            return StatsPostfixExprs(((GroupExpr *)astNode)->expr);
        case AST_ARRAY:
        {
            std::vector<Expr *> result;
            for (const auto &e : ((ArrayExpr *)astNode)->elements)
            {
                auto eResult = StatsPostfixExprs(e);
                result.insert(result.end(), eResult.begin(), eResult.end());
            }
            return result;
        }
        case AST_TABLE:
        {
            std::vector<Expr *> result;
            for (const auto &[k, v] : ((TableExpr *)astNode)->elements)
            {
                auto kResult = StatsPostfixExprs(k);
                auto vResult = StatsPostfixExprs(v);
                result.insert(result.end(), kResult.begin(), kResult.end());
                result.insert(result.end(), vResult.begin(), vResult.end());
            }
            return result;
        }
        case AST_INDEX:
        {
            std::vector<Expr *> result;
            auto dsResult = StatsPostfixExprs(((IndexExpr *)astNode)->ds);
            result.insert(result.end(), dsResult.begin(), dsResult.end());
            auto indexResult = StatsPostfixExprs(((IndexExpr *)astNode)->index);
            result.insert(result.end(), indexResult.begin(), indexResult.end());
            return result;
        }
        case AST_PREFIX:
            return StatsPostfixExprs(((PrefixExpr *)astNode)->right);
        case AST_INFIX:
        {
            std::vector<Expr *> result;
            auto leftResult = StatsPostfixExprs(((InfixExpr *)astNode)->left);
            auto rightResult = StatsPostfixExprs(((InfixExpr *)astNode)->right);

            result.insert(result.end(), leftResult.begin(), leftResult.end());
            result.insert(result.end(), rightResult.begin(), rightResult.end());
            return result;
        }
        case AST_POSTFIX:
        {
            std::vector<Expr *> result;
            auto leftResult = StatsPostfixExprs(((PostfixExpr *)astNode)->left);
            result.insert(result.end(), leftResult.begin(), leftResult.end());
            result.emplace_back((PostfixExpr *)astNode);
            return result;
        }
        case AST_CONDITION:
        {
            std::vector<Expr *> result;
            auto conditionResult = StatsPostfixExprs(((ConditionExpr *)astNode)->condition);
            result.insert(result.end(), conditionResult.begin(), conditionResult.end());
            auto trueBranchResult = StatsPostfixExprs(((ConditionExpr *)astNode)->trueBranch);
            result.insert(result.end(), trueBranchResult.begin(), trueBranchResult.end());
            auto falseBranchResult = StatsPostfixExprs(((ConditionExpr *)astNode)->falseBranch);
            result.insert(result.end(), falseBranchResult.begin(), falseBranchResult.end());
            return result;
        }
        case AST_LAMBDA:
        {
            std::vector<Expr *> result;
            for (const auto &param : ((LambdaExpr *)astNode)->parameters)
            {
                auto paramResult = StatsPostfixExprs(param);
                result.insert(result.end(), paramResult.begin(), paramResult.end());
            }
            auto bodyResult = StatsPostfixExprs(((LambdaExpr *)astNode)->body);
            result.insert(result.end(), bodyResult.begin(), bodyResult.end());
            return result;
        }
        case AST_CALL:
        {
            std::vector<Expr *> result;
            auto calleeResult = StatsPostfixExprs(((CallExpr *)astNode)->callee);
            result.insert(result.end(), calleeResult.begin(), calleeResult.end());
            for (const auto &argument : ((CallExpr *)astNode)->arguments)
            {
                auto argumentResult = StatsPostfixExprs(argument);
                result.insert(result.end(), argumentResult.begin(), argumentResult.end());
            }
            return result;
        }
        case AST_REF:
            return StatsPostfixExprs(((RefExpr *)astNode)->refExpr);
        default:
            return {};
        }
        return {};
    }
}