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
		if (stmt->type == AST_ASTSTMTS)
		{
			auto stmts = ((AstStmts *)stmt)->stmts;
			for (const auto &s : stmts)
				CompileDecl(s);
		}
		else
			CompileDecl(stmt);

		EmitReturn(0, stmt->tagToken);

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
		symbol->type = SymbolType::LOCAL;
		symbol->index = mSymbolTable->mLocalSymbolCount++;
		symbol->descType = ValueDesc::CONSTANT;
		symbol->scopeDepth = 0;
		symbol->name = L"_main_start_up";

		for (const auto &libName : LibraryManager::Instance().mStdLibraryMap)
			mSymbolTable->Define(Token(), ValueDesc::CONSTANT, libName);
	}

	void Compiler::CompileDecl(Stmt *stmt)
	{
		int64_t breakStmtAddress = -1;	  // useless
		int64_t contineuStmtAddress = -1; // useless
		CompileDecl(stmt, breakStmtAddress, contineuStmtAddress);
	}

	void Compiler::CompileDecl(Stmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress)
	{
		switch (stmt->type)
		{
		case AST_VAR:
			CompileVarDecl((VarStmt *)stmt);
			break;
		case AST_FUNCTION:
			CompileFunctionDecl((FunctionStmt *)stmt);
			break;
		case AST_CLASS:
			CompileClassDecl((ClassStmt *)stmt);
			break;
		case AST_ENUM:
			CompileEnumDecl((EnumStmt *)stmt);
			break;
		default:
			CompileStmt((Stmt *)stmt, breakStmtAddress, continueStmtAddress);
			break;
		}
	}
	void Compiler::CompileVarDecl(VarStmt *stmt)
	{
		ValueDesc valueDesc;

		if (stmt->privilege == VarStmt::Privilege ::MUTABLE)
			valueDesc = ValueDesc::VARIABLE;
		else if (stmt->privilege == VarStmt::Privilege ::IMMUTABLE)
			valueDesc = ValueDesc::CONSTANT;

		auto postfixExprs = StatsPostfixExprs(stmt);

		{
			for (const auto &[k, v] : stmt->variables)
			{
				//destructuring assignment like let [x,y,...args]=....
				if (k->type == AST_ARRAY)
				{
					auto arrayExpr = (ArrayExpr *)k;

					uint32_t resolveAddress = 0;
					OpCode appregateOpCode = OP_APPREGATE_RESOLVE;
					uint32_t appregateOpCodeAddress;

					//compile right value
					{
						CompileExpr(v);

						appregateOpCodeAddress = EmitOpCode((OpCode)0xFF, arrayExpr->tagToken) - 1;
						resolveAddress = Emit((OpCode)0xFF);
					}

					int32_t resolveCount = 0;

					if (mSymbolTable->enclosing == nullptr && mSymbolTable->mScopeDepth > 0) //local scope
						std::reverse(arrayExpr->elements.begin(), arrayExpr->elements.end());

					for (int32_t i = 0; i < arrayExpr->elements.size(); ++i)
					{
						Symbol symbol;

						if (((VarDescExpr *)arrayExpr->elements[i])->name->type == AST_IDENTIFIER)
						{
							auto literal = ((IdentifierExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->literal;
							auto token = ((IdentifierExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->tagToken;
							symbol = mSymbolTable->Define(token, valueDesc, literal);
							resolveCount++;
						}
						else if (((VarDescExpr *)arrayExpr->elements[i])->name->type == AST_VAR_ARG)
						{
							//varArg with name like:let [x,y,...args] (means IdentifierExpr* in VarDescExpr* not nullptr)
							if (((VarArgExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->argName)
							{
								auto literal = ((VarArgExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->argName->literal;
								auto token = ((VarArgExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->argName->tagToken;
								symbol = mSymbolTable->Define(token, valueDesc, literal);
								resolveCount++;
								appregateOpCode = OP_APPREGATE_RESOLVE_VAR_ARG;
							}
							else // var arg without names like: let [x,y,...]
								continue;
						}

						if (symbol.type == SymbolType::GLOBAL)
						{
							EmitOpCode(OP_SET_GLOBAL, symbol.relatedToken);
							Emit(symbol.index);
							EmitOpCode(OP_POP, symbol.relatedToken);
						}
					}

					CurOpCodes()[appregateOpCodeAddress] = appregateOpCode;
					CurOpCodes()[resolveAddress] = resolveCount;
				}
				else if (k->type == AST_VAR_DESC)
				{
					CompileExpr(v);

					std::wstring literal;
					Token token;
					if (((VarDescExpr *)k)->name->type == AST_IDENTIFIER)
					{
						literal = ((IdentifierExpr *)(((VarDescExpr *)k)->name))->literal;
						token = ((IdentifierExpr *)(((VarDescExpr *)k)->name))->tagToken;
					}

					else if (((VarDescExpr *)k)->name->type == AST_VAR_ARG)
					{
						literal = ((VarArgExpr *)((VarDescExpr *)k)->name)->argName->literal;
						token = ((VarArgExpr *)((VarDescExpr *)k)->name)->argName->tagToken;
					}

					auto symbol = mSymbolTable->Define(token, valueDesc, literal);
					if (symbol.type == SymbolType::GLOBAL)
					{
						EmitOpCode(OP_SET_GLOBAL, symbol.relatedToken);
						Emit(symbol.index);
						EmitOpCode(OP_POP, symbol.relatedToken);
					}
				}
				else
					Hint::Error(k->tagToken, L"Unknown variable:{}", k->ToString());
			}
		}

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}
	}

	void Compiler::CompileFunctionDecl(FunctionStmt *stmt)
	{
		auto symbol = CompileFunction(stmt);
		if (symbol.type == SymbolType::GLOBAL)
		{
			EmitOpCode(OP_SET_GLOBAL, symbol.relatedToken);
			Emit(symbol.index);
		}
		else if (symbol.type == SymbolType::LOCAL)
		{
			EmitOpCode(OP_SET_LOCAL, symbol.relatedToken);
			Emit(symbol.index);
		}
		EmitOpCode(OP_POP, symbol.relatedToken);
	}
	void Compiler::CompileClassDecl(ClassStmt *stmt)
	{
		auto symbol = mSymbolTable->Define(stmt->tagToken, ValueDesc::CONSTANT, stmt->name);

		mFunctionList.emplace_back(new FunctionObject(stmt->name));
		mSymbolTable = new SymbolTable(mSymbolTable);

		EnterScope();

		int8_t varCount = 0;
		int8_t constCount = 0;
		for (const auto &varStmt : stmt->varStmts)
		{
			for (const auto &[k, v] : varStmt->variables)
			{
				if (varStmt->privilege != VarStmt::Privilege::MUTABLE)
					continue;

				CompileExpr(v);

				if (k->type == AST_ARRAY)
				{
					auto arrayExpr = (ArrayExpr *)k;

					for (int32_t i = 0; i < arrayExpr->elements.size(); ++i)
					{
						std::wstring literal;
						Token token;
						if (((VarDescExpr *)arrayExpr->elements[i])->name->type == AST_IDENTIFIER)
						{
							auto identExpr = ((IdentifierExpr *)((VarDescExpr *)arrayExpr->elements[i])->name);
							literal = identExpr->literal;
							token = identExpr->tagToken;
						}
						else if (((VarDescExpr *)arrayExpr->elements[i])->name->type == AST_VAR_ARG)
						{
							auto varArgExpr = ((VarArgExpr *)((VarDescExpr *)arrayExpr->elements[i])->name);
							literal = varArgExpr->argName->literal;
							token = varArgExpr->argName->tagToken;
						}

						EmitConstant(new StrObject(literal), token);
					}
				}
				else if (k->type == AST_VAR_DESC)
				{
					std::wstring literal;
					Token token;
					if (((VarDescExpr *)k)->name->type == AST_IDENTIFIER)
					{
						auto identExpr = ((IdentifierExpr *)(((VarDescExpr *)k)->name));
						literal = identExpr->literal;
						token = identExpr->tagToken;
					}
					else if (((VarDescExpr *)k)->name->type == AST_VAR_ARG)
					{
						auto varArgExpr = ((VarArgExpr *)((VarDescExpr *)k)->name);
						literal = varArgExpr->argName->literal;
						token = varArgExpr->tagToken;
					}

					EmitConstant(new StrObject(literal), token);
				}

				varCount++;
			}
		}

		for (const auto &varStmt : stmt->varStmts)
		{
			for (const auto &[k, v] : varStmt->variables)
			{
				if (varStmt->privilege != VarStmt::Privilege::IMMUTABLE)
					continue;

				CompileExpr(v);

				if (k->type == AST_ARRAY)
				{
					auto arrayExpr = (ArrayExpr *)k;

					for (int32_t i = 0; i < arrayExpr->elements.size(); ++i)
					{
						std::wstring literal;
						Token token;
						if (((VarDescExpr *)arrayExpr->elements[i])->name->type == AST_IDENTIFIER)
						{
							auto identExpr = ((IdentifierExpr *)((VarDescExpr *)arrayExpr->elements[i])->name);
							literal = identExpr->literal;
							token = identExpr->tagToken;
						}
						else if (((VarDescExpr *)arrayExpr->elements[i])->name->type == AST_VAR_ARG)
						{
							auto varArgExpr = ((VarArgExpr *)((VarDescExpr *)arrayExpr->elements[i])->name);
							literal = varArgExpr->argName->literal;
							token = varArgExpr->argName->tagToken;
						}

						EmitConstant(new StrObject(literal), token);
					}
				}
				else if (k->type == AST_VAR_DESC)
				{
					std::wstring literal;
					Token token;
					if (((VarDescExpr *)k)->name->type == AST_IDENTIFIER)
					{
						auto identExpr = ((IdentifierExpr *)(((VarDescExpr *)k)->name));
						literal = identExpr->literal;
						token = identExpr->tagToken;
					}
					else if (((VarDescExpr *)k)->name->type == AST_VAR_ARG)
					{
						auto varArgExpr = ((VarArgExpr *)((VarDescExpr *)k)->name);
						literal = varArgExpr->argName->literal;
						token = varArgExpr->tagToken;
					}

					EmitConstant(new StrObject(literal), token);
				}

				constCount++;
			}
		}

		for (const auto &fnStmt : stmt->fnStmts)
		{
			CompileFunction(fnStmt);
			EmitConstant(new StrObject(fnStmt->name->literal), fnStmt->tagToken);
			constCount++;
		}

		for (const auto &parentClass : stmt->parentClasses)
		{
			CompileIdentifierExpr(parentClass, RWState::READ);
			EmitOpCode(OP_CALL, parentClass->tagToken);
			Emit(0);
			EmitConstant(new StrObject(parentClass->literal), parentClass->tagToken);
		}

		for (const auto &ctor : stmt->constructors)
			CompileFunction(ctor);

		EmitConstant(new StrObject(stmt->name), stmt->tagToken);
		EmitOpCode(OP_CLASS, stmt->tagToken);
		Emit(stmt->constructors.size());
		Emit(varCount);
		Emit(constCount);
		Emit(stmt->parentClasses.size());

		EmitReturn(1, stmt->tagToken);

		ExitScope();
		mSymbolTable = mSymbolTable->enclosing;

		auto function = mFunctionList.back();
		mFunctionList.pop_back();

		EmitClosure(function, stmt->tagToken);

		if (symbol.type == SymbolType::GLOBAL)
		{
			EmitOpCode(OP_SET_GLOBAL, symbol.relatedToken);
			Emit(symbol.index);
		}
		else if (symbol.type == SymbolType::LOCAL)
		{
			EmitOpCode(OP_SET_LOCAL, symbol.relatedToken);
			Emit(symbol.index);
		}
		EmitOpCode(OP_POP, symbol.relatedToken);
	}

	void Compiler::CompileEnumDecl(EnumStmt *stmt)
	{
		std::unordered_map<std::wstring, Value> pairs;
		for (const auto &[k, v] : stmt->enumItems)
		{
			Value enumValue;
			if (v->type == AST_INT)
				enumValue = ((IntNumExpr *)v)->value;
			else if (v->type == AST_REAL)
				enumValue = ((RealNumExpr *)v)->value;
			else if (v->type == AST_BOOL)
				enumValue = ((BoolExpr *)v)->value;
			else if (v->type == AST_STR)
				enumValue = new StrObject(((StrExpr *)v)->value);
			else
				Hint::Error(v->tagToken, L"Enum value only integer num,floating point num,boolean or string is available.");

			pairs[k->literal] = enumValue;
		}

		EmitConstant(new EnumObject(stmt->enumName->literal, pairs), stmt->enumName->tagToken);
		auto symbol = mSymbolTable->Define(stmt->tagToken, ValueDesc::CONSTANT, stmt->enumName->literal);
		if (symbol.type == SymbolType::GLOBAL)
		{
			EmitOpCode(OP_SET_GLOBAL, symbol.relatedToken);
			Emit(symbol.index);
			EmitOpCode(OP_POP, symbol.relatedToken);
		}
	}

	void Compiler::CompileStmt(Stmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress)
	{
		switch (stmt->type)
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
			CompileBreakStmt((BreakStmt *)stmt, breakStmtAddress);
			break;
		case AST_CONTINUE:
			CompileContinueStmt((ContinueStmt *)stmt, continueStmtAddress);
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

		EmitOpCode(OP_POP, stmt->tagToken);

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

		auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE, stmt->condition->tagToken);

		EmitOpCode(OP_POP, stmt->condition->tagToken);

		CompileDecl(stmt->thenBranch, breakStmtAddress, continueStmtAddress);

		auto jmpAddress = EmitJump(OP_JUMP, stmt->thenBranch->tagToken);

		PatchJump(jmpIfFalseAddress);

		EmitOpCode(OP_POP, stmt->thenBranch->tagToken);

		if (stmt->elseBranch)
			CompileDecl(stmt->elseBranch, breakStmtAddress, continueStmtAddress);

		PatchJump(jmpAddress);
	}
	void Compiler::CompileScopeStmt(ScopeStmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress)
	{
		for (const auto &s : stmt->stmts)
			CompileDecl(s, breakStmtAddress, continueStmtAddress);
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

		auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE, stmt->condition->tagToken);

		EmitOpCode(OP_POP, stmt->condition->tagToken);

		int64_t breakStmtAddress = -1;
		int64_t continueStmtAddress = -1;

		CompileStmt(stmt->body, breakStmtAddress, continueStmtAddress);

		if (continueStmtAddress != -1)
			PatchJump(continueStmtAddress);
		if (stmt->increment)
			CompileStmt(stmt->increment, breakStmtAddress, breakStmtAddress);

		EmitLoop(jmpAddress, stmt->tagToken);

		PatchJump(jmpIfFalseAddress);

		EmitOpCode(OP_POP, stmt->condition->tagToken);

		if (breakStmtAddress != -1)
			PatchJump(breakStmtAddress);
	}
	void Compiler::CompileReturnStmt(ReturnStmt *stmt)
	{
		auto postfixExprs = StatsPostfixExprs(stmt);

		if (stmt->expr)
		{
			CompileExpr(stmt->expr);
			EmitReturn(1, stmt->expr->tagToken);
		}
		else
			EmitReturn(0, stmt->tagToken);

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}
	}

	void Compiler::CompileBreakStmt(BreakStmt *stmt, int64_t &stmtAddress)
	{
		stmtAddress = EmitJump(OP_JUMP, stmt->tagToken);
	}
	void Compiler::CompileContinueStmt(ContinueStmt *stmt, int64_t &stmtAddress)
	{
		stmtAddress = EmitJump(OP_JUMP, stmt->tagToken);
	}

	void Compiler::CompileExpr(Expr *expr, const RWState &state, int8_t paramCount)
	{
		switch (expr->type)
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
		case AST_APPREGATE:
			CompileAppregateExpr((AppregateExpr *)expr);
			break;
		case AST_DICT:
			CompileDictExpr((DictExpr *)expr);
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
		case AST_BLOCK:
			CompileBlockExpr((BlockExpr *)expr);
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
		case AST_VAR_ARG:
			CompileVarArgExpr((VarArgExpr *)expr, state);
			break;
		case AST_FACTORIAL:
			CompileFactorialExpr((FactorialExpr *)expr);
			break;
		default:
			break;
		}
	}
	void Compiler::CompileInfixExpr(InfixExpr *expr)
	{
		if (expr->op == L"=")
		{
			if (expr->left->type == AST_ARRAY)
			{
				auto assignee = (ArrayExpr *)expr->left;

				uint32_t resolveAddress = 0;
				OpCode appregateOpCode = OP_APPREGATE_RESOLVE;
				uint32_t appregateOpCodeAddress;

				//compile right value
				{
					CompileExpr(expr->right);

					appregateOpCodeAddress = EmitOpCode((OpCode)0xFF, assignee->tagToken) - 1;
					resolveAddress = Emit((OpCode)0xFF);
				}

				uint32_t resolveCount = assignee->elements.size();

				if (assignee->elements.back()->type == AST_VAR_ARG)
				{
					if (((VarArgExpr *)assignee->elements.back())->argName)
						appregateOpCode = OP_APPREGATE_RESOLVE_VAR_ARG;
					else
						resolveCount--;
				}

				CurOpCodes()[appregateOpCodeAddress] = appregateOpCode;
				CurOpCodes()[resolveAddress] = resolveCount;

				for (int32_t i = 0; i < resolveCount; ++i)
				{
					CompileExpr(assignee->elements[i], RWState::WRITE);
					if (i < resolveCount - 1)
						EmitOpCode(OP_POP, assignee->elements[i]->tagToken);
				}
			}
			else
			{
				CompileExpr(expr->right);
				CompileExpr(expr->left, RWState::WRITE);
			}
		}
		else if (expr->op == L"&&")
		{
			// Short circuit calculation
			CompileExpr(expr->left);
			uint8_t address = EmitJump(OP_JUMP_IF_FALSE, expr->left->tagToken);
			EmitOpCode(OP_POP, expr->left->tagToken);
			CompileExpr(expr->right);
			PatchJump(address);
		}
		else if (expr->op == L"||")
		{
			CompileExpr(expr->left);
			uint8_t elseJumpAddress = EmitJump(OP_JUMP_IF_FALSE, expr->left->tagToken);
			uint8_t jumpAddress = EmitJump(OP_JUMP, expr->left->tagToken);
			PatchJump(elseJumpAddress);
			EmitOpCode(OP_POP, expr->left->tagToken);
			CompileExpr(expr->right);
			PatchJump(jumpAddress);
		}
		else
		{
			CompileExpr(expr->left);
			CompileExpr(expr->right);
			if (expr->op == L"+")
				EmitOpCode(OP_ADD, expr->tagToken);
			else if (expr->op == L"-")
				EmitOpCode(OP_SUB, expr->tagToken);
			else if (expr->op == L"*")
				EmitOpCode(OP_MUL, expr->tagToken);
			else if (expr->op == L"/")
				EmitOpCode(OP_DIV, expr->tagToken);
			else if (expr->op == L"%")
				EmitOpCode(OP_MOD, expr->tagToken);
			else if (expr->op == L"&")
				EmitOpCode(OP_BIT_AND, expr->tagToken);
			else if (expr->op == L"|")
				EmitOpCode(OP_BIT_OR, expr->tagToken);
			else if (expr->op == L"<")
				EmitOpCode(OP_LESS, expr->tagToken);
			else if (expr->op == L">")
				EmitOpCode(OP_GREATER, expr->tagToken);
			else if (expr->op == L"<<")
				EmitOpCode(OP_BIT_LEFT_SHIFT, expr->tagToken);
			else if (expr->op == L">>")
				EmitOpCode(OP_BIT_RIGHT_SHIFT, expr->tagToken);
			else if (expr->op == L"<=")
			{
				EmitOpCode(OP_GREATER, expr->tagToken);
				EmitOpCode(OP_NOT, expr->tagToken);
			}
			else if (expr->op == L">=")
			{
				EmitOpCode(OP_LESS, expr->tagToken);
				EmitOpCode(OP_NOT, expr->tagToken);
			}
			else if (expr->op == L"==")
				EmitOpCode(OP_EQUAL, expr->tagToken);
			else if (expr->op == L"!=")
			{
				EmitOpCode(OP_EQUAL, expr->tagToken);
				EmitOpCode(OP_NOT, expr->tagToken);
			}
			else if (expr->op == L"+=")
			{
				EmitOpCode(OP_ADD, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == L"-=")
			{
				EmitOpCode(OP_SUB, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == L"*=")
			{
				EmitOpCode(OP_MUL, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == L"/=")
			{
				EmitOpCode(OP_DIV, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == L"%=")
			{
				EmitOpCode(OP_MOD, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == L"&=")
			{
				EmitOpCode(OP_BIT_AND, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == L"|=")
			{
				EmitOpCode(OP_BIT_OR, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == L"<<=")
			{
				EmitOpCode(OP_BIT_LEFT_SHIFT, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == L">>=")
			{
				EmitOpCode(OP_BIT_RIGHT_SHIFT, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
		}
	}
	void Compiler::CompileIntNumExpr(IntNumExpr *expr)
	{
		EmitConstant(expr->value, expr->tagToken);
	}
	void Compiler::CompileRealNumExpr(RealNumExpr *expr)
	{
		EmitConstant(expr->value, expr->tagToken);
	}
	void Compiler::CompileBoolExpr(BoolExpr *expr)
	{
		EmitConstant(expr->value, expr->tagToken);
	}
	void Compiler::CompilePrefixExpr(PrefixExpr *expr)
	{
		CompileExpr(expr->right);
		if (expr->op == L"!")
			EmitOpCode(OP_NOT, expr->tagToken);
		else if (expr->op == L"-")
			EmitOpCode(OP_MINUS, expr->tagToken);
		else if (expr->op == L"~")
			EmitOpCode(OP_BIT_NOT, expr->tagToken);
		else if (expr->op == L"++")
		{
			while (expr->right->type == AST_PREFIX && ((PrefixExpr *)expr->right)->op == L"++" || ((PrefixExpr *)expr->right)->op == L"--")
				expr = (PrefixExpr *)expr->right;
			EmitConstant((int64_t)1, expr->tagToken);
			EmitOpCode(OP_ADD, expr->tagToken);
			CompileExpr(expr->right, RWState::WRITE);
		}
		else if (expr->op == L"--")
		{
			while (expr->right->type == AST_PREFIX && ((PrefixExpr *)expr->right)->op == L"++" || ((PrefixExpr *)expr->right)->op == L"--")
				expr = (PrefixExpr *)expr->right;
			EmitConstant((int64_t)1, expr->tagToken);
			EmitOpCode(OP_SUB, expr->tagToken);
			CompileExpr(expr->right, RWState::WRITE);
		}
		else
			Hint::Error(expr->tagToken, L"No prefix op:{}", expr->op);
	}
	void Compiler::CompilePostfixExpr(PostfixExpr *expr, const RWState &state, bool isDelayCompile)
	{
		CompileExpr(expr->left, state);
		if (!isDelayCompile)
		{
			EmitConstant((int64_t)1, expr->tagToken);
			if (expr->op == L"++")
				EmitOpCode(OP_ADD, expr->tagToken);
			else if (expr->op == L"--")
				EmitOpCode(OP_SUB, expr->tagToken);
			else
				Hint::Error(expr->tagToken, L"No postfix op:{}", expr->op);
			CompileExpr(expr->left, RWState::WRITE);
			EmitOpCode(OP_POP, expr->tagToken);
		}
	}

	void Compiler::CompileConditionExpr(ConditionExpr *expr)
	{
		CompileExpr(expr->condition);

		auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE, expr->condition->tagToken);

		EmitOpCode(OP_POP, expr->condition->tagToken);

		CompileExpr(expr->trueBranch);

		auto jmpAddress = EmitJump(OP_JUMP, expr->trueBranch->tagToken);

		PatchJump(jmpIfFalseAddress);

		EmitOpCode(OP_POP, expr->trueBranch->tagToken);

		CompileExpr(expr->falseBranch);

		PatchJump(jmpAddress);
	}

	void Compiler::CompileStrExpr(StrExpr *expr)
	{
		EmitConstant(new StrObject(expr->value), expr->tagToken);
	}
	void Compiler::CompileNullExpr(NullExpr *expr)
	{
		EmitOpCode(OP_NULL, expr->tagToken);
	}
	void Compiler::CompileGroupExpr(GroupExpr *expr)
	{
		CompileExpr(expr->expr);
	}
	void Compiler::CompileArrayExpr(ArrayExpr *expr)
	{
		for (int32_t i = expr->elements.size() - 1; i >= 0; --i)
			CompileExpr(expr->elements[i]);
		EmitOpCode(OP_ARRAY, expr->tagToken);

		uint8_t pos = expr->elements.size();
		Emit(pos);
	}

	void Compiler::CompileAppregateExpr(AppregateExpr *expr)
	{
		for (int32_t i = expr->exprs.size() - 1; i >= 0; --i)
			CompileExpr(expr->exprs[i]);
		EmitOpCode(OP_ARRAY, expr->tagToken);

		uint8_t pos = expr->exprs.size();
		Emit(pos);
	}

	void Compiler::CompileDictExpr(DictExpr *expr)
	{
		for (int32_t i = expr->elements.size() - 1; i >= 0; --i)
		{
			CompileExpr(expr->elements[i].second);
			CompileExpr(expr->elements[i].first);
		}
		EmitOpCode(OP_DICT, expr->tagToken);
		uint8_t pos = expr->elements.size();
		Emit(pos);
	}

	void Compiler::CompileIndexExpr(IndexExpr *expr, const RWState &state)
	{
		CompileExpr(expr->ds);
		CompileExpr(expr->index);
		if (state == RWState::READ)
			EmitOpCode(OP_GET_INDEX, expr->tagToken);
		else
			EmitOpCode(OP_SET_INDEX, expr->tagToken);
	}

	void Compiler::CompileNewExpr(NewExpr *expr)
	{
		if (expr->callee->type == AST_CALL)
		{
			auto callee = (CallExpr *)expr->callee;
			CompileExpr(callee->callee, RWState::READ);
			EmitOpCode(OP_CALL, expr->callee->tagToken);
			Emit(0);
			for (const auto &arg : callee->arguments)
				CompileExpr(arg);
			EmitOpCode(OP_CALL, expr->callee->tagToken);
			Emit(callee->arguments.size());
		}
		else if (expr->callee->type == AST_ANONY_OBJ)
			CompileAnonymousObjExpr((AnonyObjExpr *)(expr->callee));
	}

	void Compiler::CompileThisExpr(ThisExpr *expr)
	{
		auto identExpr = new IdentifierExpr(L"this");
		identExpr->tagToken = expr->tagToken;
		CompileExpr(identExpr);
	}

	void Compiler::CompileBaseExpr(BaseExpr *expr)
	{
		auto identExpr = new IdentifierExpr(L"this");
		identExpr->tagToken = expr->tagToken;
		CompileExpr(identExpr);
		EmitConstant(new StrObject(expr->callMember->ToString()), expr->tagToken);
		EmitOpCode(OP_GET_BASE, expr->callMember->tagToken);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state, int8_t paramCount)
	{
		OpCode getOp, setOp;
		auto symbol = mSymbolTable->Resolve(expr->tagToken, expr->literal, paramCount);
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
		else if (symbol.type == SymbolType::UPVALUE)
		{
			getOp = OP_GET_UPVALUE;
			setOp = OP_SET_UPVALUE;
		}

		if (state == RWState::WRITE)
		{
			if (symbol.descType == ValueDesc::VARIABLE)
			{
				EmitOpCode(setOp, expr->tagToken);
				if (symbol.type == SymbolType::UPVALUE)
					Emit(symbol.upvalue.index);
				else
					Emit(symbol.index);
			}
			else
				Hint::Error(expr->tagToken, L"{} is a constant,which cannot be assigned!", expr->ToString());
		}
		else
		{
			EmitOpCode(getOp, expr->tagToken);
			if (symbol.type == SymbolType::UPVALUE)
				Emit(symbol.upvalue.index);
			else
				Emit(symbol.index);
		}
	}
	void Compiler::CompileLambdaExpr(LambdaExpr *expr)
	{
		mFunctionList.emplace_back(new FunctionObject());
		mSymbolTable = new SymbolTable(mSymbolTable);

		mSymbolTable->Define(expr->tagToken, ValueDesc::CONSTANT, L"");

		CurFunction()->arity = expr->parameters.size();

		for (const auto &param : expr->parameters)
		{
			auto varDescExpr = (VarDescExpr *)param;
			if (varDescExpr->name->type == AST_IDENTIFIER)
				mSymbolTable->Define(varDescExpr->tagToken, ValueDesc::VARIABLE, ((IdentifierExpr *)((VarDescExpr *)param)->name)->literal);
			else if (varDescExpr->name->type == AST_VAR_ARG)
			{
				auto varArg = ((VarArgExpr *)varDescExpr->name);
				if (varArg->argName)
					mSymbolTable->Define(varArg->tagToken, ValueDesc::VARIABLE, varArg->argName->literal);
			}
		}

		EnterScope();

		int64_t breakStmtAddress = -1;
		int64_t continueStmtAddress = -1;
		CompileScopeStmt(expr->body, breakStmtAddress, continueStmtAddress);

		if (CurChunk().opCodes[CurChunk().opCodes.size() - 2] != OP_RETURN)
			EmitReturn(0, expr->body->stmts.back()->tagToken);

		mSymbolTable = mSymbolTable->enclosing;

		auto function = mFunctionList.back();
		mFunctionList.pop_back();

		EmitClosure(function, expr->tagToken);
	}

	void Compiler::CompileBlockExpr(BlockExpr *expr)
	{
		EnterScope();
		for (const auto &s : expr->stmts)
			CompileDecl(s);
		CompileExpr(expr->endExpr);
		ExitScope();
	}

	void Compiler::CompileCallExpr(CallExpr *expr)
	{
		CompileExpr(expr->callee, RWState::READ, expr->arguments.size());
		for (const auto &arg : expr->arguments)
			CompileExpr(arg);
		EmitOpCode(OP_CALL, expr->callee->tagToken);
		Emit(expr->arguments.size());
	}
	void Compiler::CompileDotExpr(DotExpr *expr, const RWState &state)
	{
		CompileExpr(expr->callee);
		EmitConstant(new StrObject(expr->callMember->literal), expr->callee->tagToken);
		if (state == RWState::WRITE)
			EmitOpCode(OP_SET_PROPERTY, expr->callMember->tagToken);
		else
			EmitOpCode(OP_GET_PROPERTY, expr->callMember->tagToken);
	}
	void Compiler::CompileRefExpr(RefExpr *expr)
	{
		Symbol symbol;
		if (expr->refExpr->type == AST_INDEX)
		{
			auto refIdxExpr = ((IndexExpr *)expr->refExpr);
			CompileExpr(refIdxExpr->index);
			symbol = mSymbolTable->Resolve(refIdxExpr->ds->tagToken, refIdxExpr->ds->ToString());
			if (symbol.type == SymbolType::GLOBAL)
			{
				EmitOpCode(OP_REF_INDEX_GLOBAL, symbol.relatedToken);
				Emit(symbol.index);
			}
			else if (symbol.type == SymbolType::LOCAL)
			{
				EmitOpCode(OP_REF_INDEX_LOCAL, symbol.relatedToken);
				Emit(symbol.index);
			}
			else if (symbol.type == SymbolType::UPVALUE)
			{
				EmitOpCode(OP_REF_INDEX_UPVALUE, symbol.relatedToken);
				Emit(symbol.upvalue.index);
			}
		}
		else
		{
			symbol = mSymbolTable->Resolve(expr->refExpr->tagToken, expr->refExpr->ToString());
			if (symbol.type == SymbolType::GLOBAL)
			{
				EmitOpCode(OP_REF_GLOBAL, symbol.relatedToken);
				Emit(symbol.index);
			}
			else if (symbol.type == SymbolType::LOCAL)
			{
				EmitOpCode(OP_REF_LOCAL, symbol.relatedToken);
				Emit(symbol.index);
			}
			else if (symbol.type == SymbolType::UPVALUE)
			{
				EmitOpCode(OP_REF_UPVALUE, symbol.relatedToken);
				Emit(symbol.upvalue.index);
			}
		}
	}

	void Compiler::CompileAnonymousObjExpr(AnonyObjExpr *expr)
	{
		for (auto [k, v] : expr->elements)
		{
			CompileExpr(v);
			EmitConstant(new StrObject(k), v->tagToken);
		}
		EmitOpCode(OP_ANONYMOUS_OBJ, expr->tagToken);
		uint8_t pos = expr->elements.size();
		Emit(pos);
	}

	void Compiler::CompileVarArgExpr(VarArgExpr *expr, const RWState &state)
	{
		if (expr->argName)
		{
			CompileExpr(expr->argName, state);
		}
	}

	void Compiler::CompileFactorialExpr(FactorialExpr *expr, const RWState &state)
	{
		CompileExpr(expr->expr, state);
		EmitOpCode(OP_FACTORIAL, expr->tagToken);
	}

	Symbol Compiler::CompileFunction(FunctionStmt *stmt)
	{
		auto functionSymbol = mSymbolTable->Define(stmt->tagToken, ValueDesc::CONSTANT, stmt->name->literal, stmt->parameters.size());

		mFunctionList.emplace_back(new FunctionObject(stmt->name->literal));
		mSymbolTable = new SymbolTable(mSymbolTable);

		std::wstring symbolName = L"";
		if (stmt->type == FunctionType::CLASS_CLOSURE || stmt->type == FunctionType::CLASS_CONSTRUCTOR)
			symbolName = L"this";
		mSymbolTable->Define(stmt->tagToken, ValueDesc::CONSTANT, symbolName);

		CurFunction()->arity = stmt->parameters.size();

		for (const auto &param : stmt->parameters)
		{
			auto varDescExpr = (VarDescExpr *)param;
			if (varDescExpr->name->type == AST_IDENTIFIER)
				mSymbolTable->Define(varDescExpr->tagToken, ValueDesc::VARIABLE, ((IdentifierExpr *)((VarDescExpr *)param)->name)->literal);
			else if (varDescExpr->name->type == AST_VAR_ARG)
			{
				auto varArg = ((VarArgExpr *)varDescExpr->name);
				if (varArg->argName)
					mSymbolTable->Define(varArg->argName->tagToken, ValueDesc::VARIABLE, varArg->argName->literal);
			}
		}

		EnterScope();

		int64_t breakStmtAddress = -1;
		int64_t continueStmtAddress = -1;
		CompileScopeStmt(stmt->body, breakStmtAddress, continueStmtAddress);

		if (stmt->type == FunctionType::CLASS_CONSTRUCTOR)
		{
			EmitOpCode(OP_GET_LOCAL, stmt->tagToken);
			Emit(0);
			EmitReturn(1, stmt->tagToken);
		}

		mFunctionList.back()->upValueCount = mSymbolTable->mUpValueCount;

		auto upvalues = mSymbolTable->mUpValues;

		mSymbolTable = mSymbolTable->enclosing;

		auto function = mFunctionList.back();
		mFunctionList.pop_back();

		EmitClosure(function, stmt->tagToken);

		for (int32_t i = 0; i < function->upValueCount; ++i)
		{
			Emit(upvalues[i].location);
			Emit(upvalues[i].depth);
		}

		return functionSymbol;
	}

	uint64_t Compiler::EmitOpCode(OpCode opCode, Token token)
	{
		Emit((uint8_t)opCode);

		CurChunk().opCodeRelatedTokens.emplace_back(token);

		Emit(CurChunk().opCodeRelatedTokens.size() - 1);

		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::Emit(uint8_t opcode)
	{
		CurOpCodes().emplace_back(opcode);
		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::EmitConstant(const Value &value, Token token)
	{
		EmitOpCode(OP_CONSTANT, token);
		uint8_t pos = AddConstant(value);
		Emit(pos);
		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::EmitClosure(FunctionObject *function, Token token)
	{
		uint8_t pos = AddConstant(function);
		EmitOpCode(OP_CLOSURE, token);
		Emit(pos);
		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::EmitReturn(uint8_t retCount, Token token)
	{
		EmitOpCode(OP_RETURN, token);
		Emit(retCount);
		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::EmitJump(uint8_t opcode, Token token)
	{
		EmitOpCode((OpCode)opcode, token);
		Emit(0xFF);
		Emit(0xFF);
		return CurOpCodes().size() - 2;
	}

	void Compiler::EmitLoop(uint16_t opcode, Token token)
	{
		EmitOpCode(OP_LOOP, token);
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
			Symbol *symbol = &mSymbolTable->mSymbols[i];
			if (symbol->type == SymbolType::LOCAL &&
				symbol->scopeDepth > mSymbolTable->mScopeDepth)
			{
				if (symbol->isCaptured)
					EmitOpCode(OP_CLOSE_UPVALUE, symbol->relatedToken);
				else
					EmitOpCode(OP_POP, symbol->relatedToken);
				symbol->type = SymbolType::GLOBAL; // mark as global to avoid second pop
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

		switch (astNode->type)
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
		case AST_VAR:
		{
			std::vector<Expr *> result;
			for (const auto &[k, v] : ((VarStmt *)astNode)->variables)
			{
				auto varResult = StatsPostfixExprs(v);
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

			for (const auto &varStmt : ((ClassStmt *)astNode)->varStmts)
			{
				auto letStmtResult = StatsPostfixExprs(varStmt);
				result.insert(result.end(), letStmtResult.begin(), letStmtResult.end());
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
		case AST_DICT:
		{
			std::vector<Expr *> result;
			for (const auto &[k, v] : ((DictExpr *)astNode)->elements)
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