#include "Compiler.h"
#include "Utils.h"
#include "Object.h"
#include "Library.h"
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

		EmitReturn(0);

		return CurFunction();
	}

	void Compiler::ResetStatus()
	{
		std::vector<FunctionObject *>().swap(mFunctionList);
		mFunctionList.emplace_back(new FunctionObject(L"_main_start_up"));

		if (mSymbolTable)
			delete mSymbolTable;
		mSymbolTable = new SymbolTable();

		auto symbol = &mSymbolTable->mSymbols[mSymbolTable->mSymbolCount++];
		symbol->type = SYMBOL_LOCAL;
		symbol->index = mSymbolTable->mLocalSymbolCount++;
		symbol->descType = DESC_CONSTANT;
		symbol->scopeDepth = 0;
		symbol->name = L"_main_start_up";

		for (const auto &libName : gLibraryMap)
			mSymbolTable->Define(DESC_CONSTANT, libName);
	}

	void Compiler::CompileDeclaration(Stmt *stmt)
	{
		int64_t breakStmtAddress = -1;	  //useless
		int64_t contineuStmtAddress = -1; //useless
		CompileDeclaration(stmt, breakStmtAddress, contineuStmtAddress);
	}

	void Compiler::CompileDeclaration(Stmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress)
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
		case AST_ENUM:
			CompileEnumDeclaration((EnumStmt *)stmt);
			break;
		default:
			CompileStmt((Stmt *)stmt, breakStmtAddress, continueStmtAddress);
			break;
		}
	}
	void Compiler::CompileLetDeclaration(LetStmt *stmt)
	{
		auto postfixExprs = StatsPostfixExprs(stmt);

		for (const auto &[k, v] : stmt->variables)
		{
			CompileExpr(v.value);
			auto symbol = mSymbolTable->Define(DESC_VARIABLE, k->literal);
			if (symbol.type == SYMBOL_GLOBAL)
			{
				Emit(OP_SET_GLOBAL);
				Emit(symbol.index);
				Emit(OP_POP);
			}
		}

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}
	}

	void Compiler::CompileConstDeclaration(ConstStmt *stmt)
	{
		auto postfixExprs = StatsPostfixExprs(stmt);

		for (const auto &[k, v] : stmt->consts)
		{
			CompileExpr(v.value);
			auto symbol = mSymbolTable->Define(DESC_CONSTANT, k->literal);
			if (symbol.type == SYMBOL_GLOBAL)
			{
				Emit(OP_SET_GLOBAL);
				Emit(symbol.index);
				Emit(OP_POP);
			}
		}

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}
	}

	void Compiler::CompileFunctionDeclaration(FunctionStmt *stmt)
	{
		auto symbol = CompileFunction(stmt);
		if (symbol.type == SYMBOL_GLOBAL)
		{
			Emit(OP_SET_GLOBAL);
			Emit(symbol.index);
		}
		else if (symbol.type == SYMBOL_LOCAL)
		{
			Emit(OP_SET_LOCAL);
			Emit(symbol.index);
		}
		Emit(OP_POP);
	}
	void Compiler::CompileClassDeclaration(ClassStmt *stmt)
	{
		auto symbol = mSymbolTable->Define(DESC_CONSTANT, stmt->name);

		mFunctionList.emplace_back(new FunctionObject(stmt->name));
		mSymbolTable = new SymbolTable(mSymbolTable);

		EnterScope();

		int8_t varCount = 0;
		int8_t constCount = 0;
		for (const auto &letStmt : stmt->letStmts)
		{
			for (const auto &[k, v] : letStmt->variables)
			{
				CompileExpr(v.value);
				EmitConstant(new StrObject(k->literal));
				varCount++;
			}
		}

		for (const auto &constStmt : stmt->constStmts)
		{
			for (const auto &[k, v] : constStmt->consts)
			{
				CompileExpr(v.value);
				EmitConstant(new StrObject(k->literal));
				constCount++;
			}
		}

		for (const auto &fnStmt : stmt->fnStmts)
		{
			CompileFunction(fnStmt);
			EmitConstant(new StrObject(fnStmt->name->literal));
			constCount++;
		}

		for (const auto &parentClass : stmt->parentClasses)
		{
			CompileIdentifierExpr(parentClass, RWState::READ);
			Emit(OP_CALL);
			Emit(0);
			EmitConstant(new StrObject(parentClass->literal));
		}

		for (const auto &ctor : stmt->constructors)
			CompileFunction(ctor);

		EmitConstant(new StrObject(stmt->name));
		Emit(OP_CLASS);
		Emit(stmt->constructors.size());
		Emit(varCount);
		Emit(constCount);
		Emit(stmt->parentClasses.size());

		EmitReturn(1);

		ExitScope();
		mSymbolTable = mSymbolTable->enclosing;

		auto function = mFunctionList.back();
		mFunctionList.pop_back();

		EmitClosure(function);

		if (symbol.type == SYMBOL_GLOBAL)
		{
			Emit(OP_SET_GLOBAL);
			Emit(symbol.index);
		}
		else if (symbol.type == SYMBOL_LOCAL)
		{
			Emit(OP_SET_LOCAL);
			Emit(symbol.index);
		}
		Emit(OP_POP);
	}

	void Compiler::CompileEnumDeclaration(EnumStmt *stmt)
	{
		std::unordered_map<std::wstring, Value> pairs;
		for (const auto &[k, v] : stmt->enumItems)
		{
			Value enumValue;
			if (v->Type() == AST_INT)
				enumValue = ((IntNumExpr *)v)->value;
			else if (v->Type() == AST_REAL)
				enumValue = ((RealNumExpr *)v)->value;
			else if (v->Type() == AST_BOOL)
				enumValue = ((BoolExpr *)v)->value;
			else if (v->Type() == AST_STR)
				enumValue = new StrObject(((StrExpr *)v)->value);
			else
				ASSERT(L"Enum value only integer num,floating point num,boolean or string is available")

			pairs[k->literal] = enumValue;
		}

		EmitConstant(new EnumObject(stmt->enumName->literal, pairs));
		auto symbol = mSymbolTable->Define(DESC_CONSTANT, stmt->enumName->literal);
		if (symbol.type == SYMBOL_GLOBAL)
		{
			Emit(OP_SET_GLOBAL);
			Emit(symbol.index);
			Emit(OP_POP);
		}
	}

	void Compiler::CompileStmt(Stmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress)
	{
		switch (stmt->Type())
		{
		case AST_IF:
			CompileIfStmt((IfStmt *)stmt, breakStmtAddress, continueStmtAddress);
			break;
		case AST_SCOPE:
		{
			EnterScope();
			CompileScopeStmt((ScopeStmt *)stmt, breakStmtAddress, continueStmtAddress);
			ExitScope();
			break;
		}
		case AST_WHILE:
			CompileWhileStmt((WhileStmt *)stmt);
			break;
		case AST_RETURN:
			CompileReturnStmt((ReturnStmt *)stmt);
			break;
		case AST_BREAK:
			CompileBreakStmt(breakStmtAddress);
			break;
		case AST_CONTINUE:
			CompileContinueStmt(continueStmtAddress);
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
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}
	}
	void Compiler::CompileIfStmt(IfStmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress)
	{
		auto conditionPostfixExprs = StatsPostfixExprs(stmt->condition);

		CompileExpr(stmt->condition);

		if (!conditionPostfixExprs.empty())
		{
			for (const auto &postfixExpr : conditionPostfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}

		auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE);

		Emit(OP_POP);

		CompileDeclaration(stmt->thenBranch, breakStmtAddress, continueStmtAddress);

		auto jmpAddress = EmitJump(OP_JUMP);

		PatchJump(jmpIfFalseAddress);

		Emit(OP_POP);

		if (stmt->elseBranch)
			CompileDeclaration(stmt->elseBranch, breakStmtAddress, continueStmtAddress);

		PatchJump(jmpAddress);
	}
	void Compiler::CompileScopeStmt(ScopeStmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress)
	{
		for (const auto &s : stmt->stmts)
			CompileDeclaration(s, breakStmtAddress, continueStmtAddress);
	}
	void Compiler::CompileWhileStmt(WhileStmt *stmt)
	{
		uint16_t jmpAddress = CurOpCodes().size();

		auto conditionPostfixExprs = StatsPostfixExprs(stmt->condition);

		CompileExpr(stmt->condition);

		if (!conditionPostfixExprs.empty())
		{
			for (const auto &postfixExpr : conditionPostfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}

		auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE);

		Emit(OP_POP);

		int64_t breakStmtAddress = -1;
		int64_t continueStmtAddress = -1;

		CompileStmt(stmt->body, breakStmtAddress, continueStmtAddress);

		if (continueStmtAddress != -1)
			PatchJump(continueStmtAddress);
		if (stmt->increment)
			CompileStmt(stmt->increment, breakStmtAddress, breakStmtAddress);

		EmitLoop(jmpAddress);

		PatchJump(jmpIfFalseAddress);

		Emit(OP_POP);

		if (breakStmtAddress != -1)
			PatchJump(breakStmtAddress);
	}
	void Compiler::CompileReturnStmt(ReturnStmt *stmt)
	{
		auto postfixExprs = StatsPostfixExprs(stmt->expr);

		if (stmt->expr)
		{
			CompileExpr(stmt->expr);
			EmitReturn(1);
		}
		else
			EmitReturn(0);

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}
	}

	void Compiler::CompileBreakStmt(int64_t &stmtAddress)
	{
		stmtAddress = EmitJump(OP_JUMP);
	}
	void Compiler::CompileContinueStmt(int64_t &stmtAddress)
	{
		stmtAddress = EmitJump(OP_JUMP);
	}

	void Compiler::CompileExpr(Expr *expr, const RWState &state, int8_t paramCount)
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
			CompilePostfixExpr((PostfixExpr *)expr, state);
			break;
		case AST_CONDITION:
			CompileConditionExpr((ConditionExpr *)expr);
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
			CompileIndexExpr((IndexExpr *)expr, state);
			break;
		case AST_IDENTIFIER:
			CompileIdentifierExpr((IdentifierExpr *)expr, state, paramCount);
			break;
		case AST_LAMBDA:
			CompileLambdaExpr((LambdaExpr *)expr);
			break;
		case AST_CALL:
			CompileCallExpr((CallExpr *)expr);
			break;
		case AST_DOT:
			CompileDotExpr((DotExpr *)expr, state);
			break;
		case AST_REF:
			CompileRefExpr((RefExpr *)expr);
			break;
		case AST_NEW:
			CompileNewExpr((NewExpr *)expr);
			break;
		case AST_THIS:
			CompileThisExpr((ThisExpr *)expr);
			break;
		case AST_BASE:
			CompileBaseExpr((BaseExpr *)expr);
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
			// Short circuit calculation
			CompileExpr(expr->left);
			uint8_t address = EmitJump(OP_JUMP_IF_FALSE);
			Emit(OP_POP);
			CompileExpr(expr->right);
			PatchJump(address);
		}
		else if (expr->op == L"||")
		{
			CompileExpr(expr->left);
			uint8_t elseJumpAddress = EmitJump(OP_JUMP_IF_FALSE);
			uint8_t jumpAddress = EmitJump(OP_JUMP);
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
		}
		else if (expr->op == L"--")
		{
			while (expr->right->Type() == AST_PREFIX && ((PrefixExpr *)expr->right)->op == L"++" || ((PrefixExpr *)expr->right)->op == L"--")
				expr = (PrefixExpr *)expr->right;
			EmitConstant((int64_t)1);
			Emit(OP_SUB);
			CompileExpr(expr->right, RWState::WRITE);
		}
		else
			ASSERT(L"No prefix op:" + expr->op)
	}
	void Compiler::CompilePostfixExpr(PostfixExpr *expr, const RWState &state, bool isDelayCompile)
	{
		CompileExpr(expr->left, state);
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
				ASSERT(L"No postfix op:" + expr->op)
			CompileExpr(expr->left, RWState::WRITE);
			Emit(OP_POP);
		}
	}

	void Compiler::CompileConditionExpr(ConditionExpr *expr)
	{
		CompileExpr(expr->condition);

		auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE);

		Emit(OP_POP);

		CompileExpr(expr->trueBranch);

		auto jmpAddress = EmitJump(OP_JUMP);

		PatchJump(jmpIfFalseAddress);

		Emit(OP_POP);

		CompileExpr(expr->falseBranch);

		PatchJump(jmpAddress);
	}

	void Compiler::CompileStrExpr(StrExpr *expr)
	{
		EmitConstant(new StrObject(expr->value));
	}
	void Compiler::CompileNullExpr(NullExpr *expr)
	{
		Emit(OP_NULL);
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

		uint8_t pos = expr->elements.size();
		Emit(pos);
	}

	void Compiler::CompileTableExpr(TableExpr *expr)
	{
		for (const auto &[k, v] : expr->elements)
		{
			CompileExpr(v);
			CompileExpr(k);
		}
		Emit(OP_TABLE);
		uint8_t pos = expr->elements.size();
		Emit(pos);
	}

	void Compiler::CompileIndexExpr(IndexExpr *expr, const RWState &state)
	{
		CompileExpr(expr->ds);
		CompileExpr(expr->index);
		if (state == RWState::READ)
			Emit(OP_GET_INDEX);
		else
			Emit(OP_SET_INDEX);
	}

	void Compiler::CompileNewExpr(NewExpr *expr)
	{
		auto callee = expr->callee;
		CompileExpr(callee->callee, RWState::READ);
		Emit(OP_CALL);
		Emit(0);
		for (const auto &arg : callee->arguments)
			CompileExpr(arg);
		Emit(OP_CALL);
		Emit(callee->arguments.size());
	}

	void Compiler::CompileThisExpr(ThisExpr *expr)
	{
		CompileExpr(new IdentifierExpr(L"this"));
	}

	void Compiler::CompileBaseExpr(BaseExpr *expr)
	{
		CompileExpr(new IdentifierExpr(L"this"));
		EmitConstant(new StrObject(expr->callMember->Stringify()));
		Emit(OP_GET_BASE);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state, int8_t paramCount)
	{
		OpCode getOp, setOp;
		auto symbol = mSymbolTable->Resolve(expr->literal, paramCount);
		if (symbol.type == SYMBOL_GLOBAL)
		{
			getOp = OP_GET_GLOBAL;
			setOp = OP_SET_GLOBAL;
		}
		else if (symbol.type == SYMBOL_LOCAL)
		{
			getOp = OP_GET_LOCAL;
			setOp = OP_SET_LOCAL;
		}
		else if (symbol.type == SYMBOL_UPVALUE)
		{
			getOp = OP_GET_UPVALUE;
			setOp = OP_SET_UPVALUE;
		}

		if (state == RWState::WRITE)
		{
			if (symbol.descType == DESC_VARIABLE)
			{
				Emit(setOp);
				if (symbol.type == SYMBOL_UPVALUE)
					Emit(symbol.upvalue.index);
				else
					Emit(symbol.index);
			}
			else
				ASSERT(expr->Stringify() + L"is a constant,which cannot be assigned!")
		}
		else
		{
			Emit(getOp);
			if (symbol.type == SYMBOL_UPVALUE)
				Emit(symbol.upvalue.index);
			else
				Emit(symbol.index);
		}
	}
	void Compiler::CompileLambdaExpr(LambdaExpr *expr)
	{
		mFunctionList.emplace_back(new FunctionObject());
		mSymbolTable = new SymbolTable(mSymbolTable);

		mSymbolTable->Define(DESC_CONSTANT, L"");

		CurFunction()->arity = expr->parameters.size();

		for (const auto &param : expr->parameters)
			mSymbolTable->Define(DESC_VARIABLE, param->literal);

		EnterScope();

		int64_t breakStmtAddress = -1;
		int64_t continueStmtAddress = -1;
		CompileScopeStmt(expr->body, breakStmtAddress, continueStmtAddress);

		if (CurChunk().opCodes[CurChunk().opCodes.size() - 2] != OP_RETURN)
			EmitReturn(0);

		mSymbolTable = mSymbolTable->enclosing;

		auto function = mFunctionList.back();
		mFunctionList.pop_back();

		EmitClosure(function);
	}
	void Compiler::CompileCallExpr(CallExpr *expr)
	{
		CompileExpr(expr->callee, RWState::READ, expr->arguments.size());
		for (const auto &arg : expr->arguments)
			CompileExpr(arg);
		Emit(OP_CALL);
		Emit(expr->arguments.size());
	}
	void Compiler::CompileDotExpr(DotExpr *expr, const RWState &state)
	{
		CompileExpr(expr->callee);
		EmitConstant(new StrObject(expr->callMember->literal));
		if (state == RWState::WRITE)
			Emit(OP_SET_PROPERTY);
		else
			Emit(OP_GET_PROPERTY);
	}
	void Compiler::CompileRefExpr(RefExpr *expr)
	{
		Symbol symbol;
		if (expr->refExpr->Type() == AST_INDEX)
		{
			CompileExpr(((IndexExpr *)expr->refExpr)->index);
			symbol = mSymbolTable->Resolve(((IndexExpr *)expr->refExpr)->ds->Stringify());
			if (symbol.type == SYMBOL_GLOBAL)
			{
				Emit(OP_REF_INDEX_GLOBAL);
				Emit(symbol.index);
			}
			else if (symbol.type == SYMBOL_LOCAL)
			{
				Emit(OP_REF_INDEX_LOCAL);
				Emit(symbol.index);
			}
			else if (symbol.type == SYMBOL_UPVALUE)
			{
				Emit(OP_REF_INDEX_UPVALUE);
				Emit(symbol.upvalue.index);
			}
		}
		else
		{
			symbol = mSymbolTable->Resolve(expr->refExpr->Stringify());
			if (symbol.type == SYMBOL_GLOBAL)
			{
				Emit(OP_REF_GLOBAL);
				Emit(symbol.index);
			}
			else if (symbol.type == SYMBOL_LOCAL)
			{
				Emit(OP_REF_LOCAL);
				Emit(symbol.index);
			}
			else if (symbol.type == SYMBOL_UPVALUE)
			{
				Emit(OP_REF_UPVALUE);
				Emit(symbol.upvalue.index);
			}
		}
	}

	Symbol Compiler::CompileFunction(FunctionStmt *stmt)
	{
		auto functionSymbol = mSymbolTable->Define(DESC_CONSTANT, stmt->name->literal, stmt->parameters.size());

		mFunctionList.emplace_back(new FunctionObject(stmt->name->literal));
		mSymbolTable = new SymbolTable(mSymbolTable);

		std::wstring symbolName = L"";
		if (stmt->type == FunctionType::CLASS_CLOSURE || stmt->type == FunctionType::CLASS_CONSTRUCTOR)
			symbolName = L"this";
		mSymbolTable->Define(DESC_CONSTANT, symbolName);

		CurFunction()->arity = stmt->parameters.size();

		for (const auto &param : stmt->parameters)
			mSymbolTable->Define(DESC_VARIABLE, param->literal);

		EnterScope();

		int64_t breakStmtAddress = -1;
		int64_t continueStmtAddress = -1;
		CompileScopeStmt(stmt->body, breakStmtAddress, continueStmtAddress);

		if (stmt->type == FunctionType::CLASS_CONSTRUCTOR)
		{
			Emit(OP_GET_LOCAL);
			Emit(0);
			EmitReturn(1);
		}

		if (CurChunk().opCodes[CurChunk().opCodes.size() - 2] != OP_RETURN)
			EmitReturn(0);

		mFunctionList.back()->upValueCount = mSymbolTable->mUpValueCount;

		auto upvalues = mSymbolTable->mUpValues;

		mSymbolTable = mSymbolTable->enclosing;

		auto function = mFunctionList.back();
		mFunctionList.pop_back();

		EmitClosure(function);

		for (int32_t i = 0; i < function->upValueCount; ++i)
		{
			Emit(upvalues[i].location);
			Emit(upvalues[i].depth);
		}

		return functionSymbol;
	}

	uint8_t Compiler::Emit(uint8_t opcode)
	{
		CurOpCodes().emplace_back(opcode);
		return CurOpCodes().size() - 1;
	}

	uint8_t Compiler::EmitConstant(const Value &value)
	{
		Emit(OP_CONSTANT);
		uint8_t pos = AddConstant(value);
		Emit(pos);
		return CurOpCodes().size() - 1;
	}

	uint8_t Compiler::EmitClosure(FunctionObject *function)
	{
		uint8_t pos = AddConstant(function);
		Emit(OP_CLOSURE);
		Emit(pos);
		return CurOpCodes().size() - 1;
	}

	uint8_t Compiler::EmitReturn(uint8_t retCount)
	{
		Emit(OP_RETURN);
		Emit(retCount);
		return CurOpCodes().size() - 1;
	}

	uint8_t Compiler::EmitJump(uint8_t opcode)
	{
		Emit(opcode);
		Emit(0xFF);
		Emit(0xFF);
		return CurOpCodes().size() - 2;
	}

	void Compiler::EmitLoop(uint16_t opcode)
	{
		Emit(OP_LOOP);
		uint16_t offset = CurOpCodes().size() - opcode + 2;

		Emit((offset >> 8) & 0xFF);
		Emit(offset & 0xFF);
	}

	void Compiler::PatchJump(uint8_t offset)
	{
		uint16_t jumpOffset = CurOpCodes().size() - offset - 2;
		CurOpCodes()[offset] = (jumpOffset >> 8) & 0xFF;
		CurOpCodes()[offset + 1] = (jumpOffset)&0xFF;
	}

	uint8_t Compiler::AddConstant(const Value &value)
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
			if (mSymbolTable->mSymbols[i].type == SYMBOL_LOCAL &&
				mSymbolTable->mSymbols[i].scopeDepth > mSymbolTable->mScopeDepth)
			{
				if (mSymbolTable->mSymbols[i].isCaptured)
					Emit(OP_CLOSE_UPVALUE);
				else
					Emit(OP_POP);
				mSymbolTable->mSymbols[i].type = SYMBOL_GLOBAL; // mark as global to avoid second pop
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
		if (!astNode) // check astnode is nullptr
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