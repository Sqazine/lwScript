#include "Compiler.h"
#include "Utils.h"
#include "Object.h"
#include "LibraryManager.h"
#include "Logger.h"
namespace lwscript
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
		ResetStatus();

		if (stmt->kind == AstKind::ASTSTMTS)
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
		mCurContinueStmtAddress = -1;
		mCurBreakStmtAddress = -1;

		std::vector<FunctionObject *>().swap(mFunctionList);
		mFunctionList.emplace_back(new FunctionObject(MAIN_ENTRY_FUNCTION_NAME));

		SAFE_DELETE(mSymbolTable);
		mSymbolTable = new SymbolTable();

		auto symbol = &mSymbolTable->mSymbols[mSymbolTable->mSymbolCount++];
		symbol->location = SymbolLocation::LOCAL;
		symbol->index = mSymbolTable->mLocalSymbolCount++;
		symbol->privilege = Privilege::IMMUTABLE;
		symbol->scopeDepth = 0;
		symbol->name = MAIN_ENTRY_FUNCTION_NAME;

		for (const auto &lib : LibraryManager::GetInstance()->GetLibraries())
			mSymbolTable->Define(new Token(), Privilege::IMMUTABLE, lib->name);
	}

	void Compiler::CompileDecl(Stmt *stmt)
	{
		switch (stmt->kind)
		{
		case AstKind::VAR:
			CompileVarDecl((VarStmt *)stmt);
			break;
		case AstKind::FUNCTION:
			CompileFunctionDecl((FunctionStmt *)stmt);
			break;
		case AstKind::CLASS:
			CompileClassDecl((ClassStmt *)stmt);
			break;
		case AstKind::ENUM:
			CompileEnumDecl((EnumStmt *)stmt);
			break;
		case AstKind::MODULE:
			CompileModuleDecl((ModuleStmt *)stmt);
			break;
		default:
			CompileStmt((Stmt *)stmt);
			break;
		}
	}
	void Compiler::CompileVarDecl(VarStmt *stmt)
	{
		CompileVars(stmt, false);
	}

	void Compiler::CompileFunctionDecl(FunctionStmt *stmt)
	{
		auto symbol = CompileFunction(stmt);
		EmitSymbol(symbol);
	}
	void Compiler::CompileClassDecl(ClassStmt *stmt)
	{
		auto symbol = CompileClass(stmt);
		EmitSymbol(symbol);
	}

	void Compiler::CompileEnumDecl(EnumStmt *stmt)
	{
		std::unordered_map<STD_STRING, Value> pairs;
		for (const auto &[k, v] : stmt->enumItems)
		{
			Value enumValue;
			if (v->kind == AstKind::LITERAL)
			{
				LiteralExpr *literalExpr = (LiteralExpr *)v;
				if (literalExpr->type.IsInteger())
					enumValue = literalExpr->i64Value;
				else if (literalExpr->type.IsFloating())
					enumValue = literalExpr->f64Value;
				else if (literalExpr->type.IsBoolean())
					enumValue = literalExpr->boolean;
				else if (literalExpr->type.IsString())
					enumValue = new StrObject(literalExpr->str);
				else if (literalExpr->type.IsChar())
				{
					// TODO...
				}
			}
			else
				Logger::Error(v->tagToken, TEXT("Enum value only integer num,floating point num,boolean or string is available."));

			pairs[k->literal] = enumValue;
		}

		EmitConstant(new EnumObject(stmt->name->literal, pairs), stmt->name->tagToken);
		auto symbol = mSymbolTable->Define(stmt->tagToken, Privilege::IMMUTABLE, stmt->name->literal);
		EmitSymbol(symbol);
	}

	void Compiler::CompileModuleDecl(ModuleStmt *stmt)
	{
		auto symbol = mSymbolTable->Define(stmt->tagToken, Privilege::IMMUTABLE, stmt->name->literal);

		mFunctionList.emplace_back(new FunctionObject(stmt->name->literal));

		mSymbolTable = new SymbolTable(mSymbolTable);

		mSymbolTable->Define(stmt->tagToken, Privilege::IMMUTABLE, TEXT(""));

		uint8_t constCount = 0;
		uint8_t varCount = 0;

		for (const auto &enumStmt : stmt->enumItems)
		{
			CompileEnumDecl(enumStmt);
			EmitConstant(new StrObject(enumStmt->name->literal), enumStmt->tagToken);
			constCount++;
		}

		for (const auto &fnStmt : stmt->functionItems)
		{
			CompileFunctionDecl(fnStmt);
			EmitConstant(new StrObject(fnStmt->name->literal), fnStmt->tagToken);
			constCount++;
		}

		for (const auto &classStmt : stmt->classItems)
		{
			CompileClassDecl(classStmt);
			EmitConstant(new StrObject(classStmt->name), classStmt->tagToken);
			constCount++;
		}

		for (const auto &moduleStmt : stmt->moduleItems)
		{
			CompileModuleDecl(moduleStmt);
			EmitConstant(new StrObject(moduleStmt->name->literal), moduleStmt->tagToken);
			constCount++;
		}

		for (const auto &varStmt : stmt->varItems)
		{
			if (varStmt->privilege == Privilege::IMMUTABLE)
				constCount += CompileVars(varStmt, true);
		}

		for (const auto &varStmt : stmt->varItems)
		{
			if (varStmt->privilege == Privilege::MUTABLE)
				varCount += CompileVars(varStmt, true);
		}

		EmitConstant(new StrObject(symbol.name), symbol.relatedToken);

		EmitOpCode(OP_MODULE, stmt->tagToken);
		Emit(varCount);
		Emit(constCount);

		EmitReturn(1, stmt->tagToken);

		auto function = mFunctionList.back();
		function->arity = mSymbolTable->mSymbolCount;

		mFunctionList.pop_back();

		EmitClosure(function, stmt->tagToken);

		for (int32_t i = 0; i < mSymbolTable->mSymbolCount; ++i)
			EmitOpCode(OP_NULL, stmt->tagToken);

		EmitOpCode(OP_CALL, stmt->tagToken);
		Emit(mSymbolTable->mSymbolCount);

		mSymbolTable = mSymbolTable->enclosing;

		EmitSymbol(symbol);
	}

	void Compiler::CompileStmt(Stmt *stmt)
	{
		switch (stmt->kind)
		{
		case AstKind::IF:
			CompileIfStmt((IfStmt *)stmt);
			break;
		case AstKind::SCOPE:
		{
			EnterScope();
			CompileScopeStmt((ScopeStmt *)stmt);
			ExitScope();
			break;
		}
		case AstKind::WHILE:
			CompileWhileStmt((WhileStmt *)stmt);
			break;
		case AstKind::RETURN:
			CompileReturnStmt((ReturnStmt *)stmt);
			break;
		case AstKind::BREAK:
			CompileBreakStmt((BreakStmt *)stmt);
			break;
		case AstKind::CONTINUE:
			CompileContinueStmt((ContinueStmt *)stmt);
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

	void Compiler::CompileIfStmt(IfStmt *stmt)
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

		CompileDecl(stmt->thenBranch);

		auto jmpAddress = EmitJump(OP_JUMP, stmt->thenBranch->tagToken);

		PatchJump(jmpIfFalseAddress);

		EmitOpCode(OP_POP, stmt->thenBranch->tagToken);

		if (stmt->elseBranch)
			CompileDecl(stmt->elseBranch);

		PatchJump(jmpAddress);
	}
	void Compiler::CompileScopeStmt(ScopeStmt *stmt)
	{
		for (const auto &s : stmt->stmts)
			CompileDecl(s);
	}
	void Compiler::CompileWhileStmt(WhileStmt *stmt)
	{
		auto jmpAddress = (uint32_t)CurOpCodes().size();

		auto conditionPostfixExprs = StatsPostfixExprs(stmt->condition);

		CompileExpr(stmt->condition);

		if (!conditionPostfixExprs.empty())
		{
			for (const auto &postfixExpr : conditionPostfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}

		auto jmpIfFalseAddress = EmitJump(OP_JUMP_IF_FALSE, stmt->condition->tagToken);

		EmitOpCode(OP_POP, stmt->condition->tagToken);

		mCurContinueStmtAddress = -1;
		mCurBreakStmtAddress = -1;

		CompileStmt(stmt->body);

		if (mCurContinueStmtAddress != -1)
			PatchJump(mCurContinueStmtAddress);
		if (stmt->increment)
			CompileStmt(stmt->increment);

		EmitLoop(jmpAddress, stmt->tagToken);

		PatchJump(jmpIfFalseAddress);

		EmitOpCode(OP_POP, stmt->condition->tagToken);

		if (mCurBreakStmtAddress != -1)
			PatchJump(mCurBreakStmtAddress);

		mCurContinueStmtAddress = -1;
		mCurBreakStmtAddress = -1;
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

	void Compiler::CompileBreakStmt(BreakStmt *stmt)
	{
		mCurBreakStmtAddress = EmitJump(OP_JUMP, stmt->tagToken);
	}
	void Compiler::CompileContinueStmt(ContinueStmt *stmt)
	{
		mCurContinueStmtAddress = EmitJump(OP_JUMP, stmt->tagToken);
	}

	void Compiler::CompileExpr(Expr *expr, const RWState &state, int8_t paramCount)
	{
		switch (expr->kind)
		{
		case AstKind::INFIX:
			CompileInfixExpr((InfixExpr *)expr);
			break;
		case AstKind::LITERAL:
			CompileLiteralExpr((LiteralExpr *)expr);
			break;
		case AstKind::PREFIX:
			CompilePrefixExpr((PrefixExpr *)expr);
			break;
		case AstKind::POSTFIX:
			CompilePostfixExpr((PostfixExpr *)expr, state);
			break;
		case AstKind::CONDITION:
			CompileConditionExpr((ConditionExpr *)expr);
			break;
		case AstKind::GROUP:
			CompileGroupExpr((GroupExpr *)expr);
			break;
		case AstKind::ARRAY:
			CompileArrayExpr((ArrayExpr *)expr);
			break;
		case AstKind::APPREGATE:
			CompileAppregateExpr((AppregateExpr *)expr);
			break;
		case AstKind::DICT:
			CompileDictExpr((DictExpr *)expr);
			break;
		case AstKind::INDEX:
			CompileIndexExpr((IndexExpr *)expr, state);
			break;
		case AstKind::IDENTIFIER:
			CompileIdentifierExpr((IdentifierExpr *)expr, state, paramCount);
			break;
		case AstKind::LAMBDA:
			CompileLambdaExpr((LambdaExpr *)expr);
			break;
		case AstKind::COMPOUND:
			CompileCompoundExpr((CompoundExpr *)expr);
			break;
		case AstKind::CALL:
			CompileCallExpr((CallExpr *)expr);
			break;
		case AstKind::DOT:
			CompileDotExpr((DotExpr *)expr, state);
			break;
		case AstKind::REF:
			CompileRefExpr((RefExpr *)expr);
			break;
		case AstKind::NEW:
			CompileNewExpr((NewExpr *)expr);
			break;
		case AstKind::THIS:
			CompileThisExpr((ThisExpr *)expr);
			break;
		case AstKind::BASE:
			CompileBaseExpr((BaseExpr *)expr);
			break;
		case AstKind::VAR_ARG:
			CompileVarArgExpr((VarArgExpr *)expr, state);
			break;
		case AstKind::FACTORIAL:
			CompileFactorialExpr((FactorialExpr *)expr);
			break;
		case AstKind::STRUCT:
			CompileStructExpr((StructExpr *)expr);
			break;
		default:
			break;
		}
	}
	void Compiler::CompileInfixExpr(InfixExpr *expr)
	{
		if (expr->op == TEXT("="))
		{
			if (expr->left->kind == AstKind::ARRAY)
			{
				auto assignee = (ArrayExpr *)expr->left;

				OpCode appregateOpCode = OP_APPREGATE_RESOLVE;

				CompileExpr(expr->right);

				uint64_t appregateOpCodeAddress = EmitOpCode((OpCode)0xFF, assignee->tagToken) - 1;
				uint64_t resolveAddress = Emit((OpCode)0xFF);

				uint8_t resolveCount = static_cast<uint8_t>(assignee->elements.size());

				if (assignee->elements.back()->kind == AstKind::VAR_ARG)
				{
					if (((VarArgExpr *)assignee->elements.back())->argName)
						appregateOpCode = OP_APPREGATE_RESOLVE_VAR_ARG;
					else
						resolveCount--;
				}
				else
				{
					if (expr->right->kind == AstKind::ARRAY)
					{
						auto assigneeElements = assignee->elements;
						auto rightElements = ((ArrayExpr *)expr->right)->elements;
						if (assigneeElements.size() < rightElements.size())
							Logger::Warn(rightElements[assigneeElements.size()]->tagToken, TEXT("variable count less than value count,ignore the rest value"));
						else if (assigneeElements.size() > rightElements.size())
							Logger::Warn(assigneeElements[rightElements.size()]->tagToken, TEXT("variable count greater than value count,rest value set as null"));
					}
				}

				CurOpCodes()[appregateOpCodeAddress] = appregateOpCode;
				CurOpCodes()[resolveAddress] = resolveCount;

				for (uint8_t i = 0; i < resolveCount; ++i)
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
		else if (expr->op == TEXT("&&"))
		{
			// Short circuit calculation
			CompileExpr(expr->left);
			uint64_t address = EmitJump(OP_JUMP_IF_FALSE, expr->left->tagToken);
			EmitOpCode(OP_POP, expr->left->tagToken);
			CompileExpr(expr->right);
			PatchJump(address);
		}
		else if (expr->op == TEXT("||"))
		{
			CompileExpr(expr->left);
			uint64_t elseJumpAddress = EmitJump(OP_JUMP_IF_FALSE, expr->left->tagToken);
			uint64_t jumpAddress = EmitJump(OP_JUMP, expr->left->tagToken);
			PatchJump(elseJumpAddress);
			EmitOpCode(OP_POP, expr->left->tagToken);
			CompileExpr(expr->right);
			PatchJump(jumpAddress);
		}
		else
		{
			CompileExpr(expr->left);
			CompileExpr(expr->right);
			if (expr->op == TEXT("+"))
				EmitOpCode(OP_ADD, expr->tagToken);
			else if (expr->op == TEXT("-"))
				EmitOpCode(OP_SUB, expr->tagToken);
			else if (expr->op == TEXT("*"))
				EmitOpCode(OP_MUL, expr->tagToken);
			else if (expr->op == TEXT("/"))
				EmitOpCode(OP_DIV, expr->tagToken);
			else if (expr->op == TEXT("%"))
				EmitOpCode(OP_MOD, expr->tagToken);
			else if (expr->op == TEXT("&"))
				EmitOpCode(OP_BIT_AND, expr->tagToken);
			else if (expr->op == TEXT("|"))
				EmitOpCode(OP_BIT_OR, expr->tagToken);
			else if (expr->op == TEXT("<"))
				EmitOpCode(OP_LESS, expr->tagToken);
			else if (expr->op == TEXT(">"))
				EmitOpCode(OP_GREATER, expr->tagToken);
			else if (expr->op == TEXT("<<"))
				EmitOpCode(OP_BIT_LEFT_SHIFT, expr->tagToken);
			else if (expr->op == TEXT(">>"))
				EmitOpCode(OP_BIT_RIGHT_SHIFT, expr->tagToken);
			else if (expr->op == TEXT("<="))
			{
				EmitOpCode(OP_GREATER, expr->tagToken);
				EmitOpCode(OP_NOT, expr->tagToken);
			}
			else if (expr->op == TEXT(">="))
			{
				EmitOpCode(OP_LESS, expr->tagToken);
				EmitOpCode(OP_NOT, expr->tagToken);
			}
			else if (expr->op == TEXT("=="))
				EmitOpCode(OP_EQUAL, expr->tagToken);
			else if (expr->op == TEXT("!="))
			{
				EmitOpCode(OP_EQUAL, expr->tagToken);
				EmitOpCode(OP_NOT, expr->tagToken);
			}
			else if (expr->op == TEXT("+="))
			{
				EmitOpCode(OP_ADD, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == TEXT("-="))
			{
				EmitOpCode(OP_SUB, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == TEXT("*="))
			{
				EmitOpCode(OP_MUL, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == TEXT("/="))
			{
				EmitOpCode(OP_DIV, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == TEXT("%="))
			{
				EmitOpCode(OP_MOD, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == TEXT("&="))
			{
				EmitOpCode(OP_BIT_AND, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == TEXT("|="))
			{
				EmitOpCode(OP_BIT_OR, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == TEXT("<<="))
			{
				EmitOpCode(OP_BIT_LEFT_SHIFT, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
			else if (expr->op == TEXT(">>="))
			{
				EmitOpCode(OP_BIT_RIGHT_SHIFT, expr->tagToken);
				CompileExpr(expr->left, RWState::WRITE);
			}
		}
	}

	void Compiler::CompileLiteralExpr(LiteralExpr *expr)
	{
		switch (expr->type.GetKind())
		{
		case TypeKind::I8:
		case TypeKind::U8:
		case TypeKind::I16:
		case TypeKind::U16:
		case TypeKind::I32:
		case TypeKind::U32:
		case TypeKind::I64:
		case TypeKind::U64:
			EmitConstant(expr->i64Value, expr->tagToken);
			break;
		case TypeKind::F32:
		case TypeKind::F64:
			EmitConstant(expr->f64Value, expr->tagToken);
			break;
		case TypeKind::BOOL:
			EmitConstant(expr->boolean, expr->tagToken);
			break;
		case TypeKind::STRING:
			EmitConstant(new StrObject(expr->str), expr->tagToken);
			break;
		case TypeKind::CHAR:
			break; // TODO:...
		default:
			EmitOpCode(OP_NULL, expr->tagToken);
			break;
		}
	}

	void Compiler::CompilePrefixExpr(PrefixExpr *expr)
	{
		CompileExpr(expr->right);
		if (expr->op == TEXT("!"))
			EmitOpCode(OP_NOT, expr->tagToken);
		else if (expr->op == TEXT("-"))
			EmitOpCode(OP_MINUS, expr->tagToken);
		else if (expr->op == TEXT("~"))
			EmitOpCode(OP_BIT_NOT, expr->tagToken);
		else if (expr->op == TEXT("++"))
		{
			while (expr->right->kind == AstKind::PREFIX && ((PrefixExpr *)expr->right)->op == TEXT("++") || ((PrefixExpr *)expr->right)->op == TEXT("--"))
				expr = (PrefixExpr *)expr->right;
			EmitConstant((int64_t)1, expr->tagToken);
			EmitOpCode(OP_ADD, expr->tagToken);
			CompileExpr(expr->right, RWState::WRITE);
		}
		else if (expr->op == TEXT("--"))
		{
			while (expr->right->kind == AstKind::PREFIX && ((PrefixExpr *)expr->right)->op == TEXT("++") || ((PrefixExpr *)expr->right)->op == TEXT("--"))
				expr = (PrefixExpr *)expr->right;
			EmitConstant((int64_t)1, expr->tagToken);
			EmitOpCode(OP_SUB, expr->tagToken);
			CompileExpr(expr->right, RWState::WRITE);
		}
		else
			Logger::Error(expr->tagToken, TEXT("No prefix op:{}"), expr->op);
	}
	void Compiler::CompilePostfixExpr(PostfixExpr *expr, const RWState &state, bool isDelayCompile)
	{
		CompileExpr(expr->left, state);
		if (!isDelayCompile)
		{
			EmitConstant((int64_t)1, expr->tagToken);
			if (expr->op == TEXT("++"))
				EmitOpCode(OP_ADD, expr->tagToken);
			else if (expr->op == TEXT("--"))
				EmitOpCode(OP_SUB, expr->tagToken);
			else
				Logger::Error(expr->tagToken, TEXT("No postfix op:{}"), expr->op);
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

	void Compiler::CompileGroupExpr(GroupExpr *expr)
	{
		CompileExpr(expr->expr);
	}
	void Compiler::CompileArrayExpr(ArrayExpr *expr)
	{
		for (const auto &elementExpr : expr->elements)
			CompileExpr(elementExpr);
		EmitOpCode(OP_ARRAY, expr->tagToken);

		uint8_t pos = (uint8_t)expr->elements.size();
		Emit(pos);
	}

	void Compiler::CompileAppregateExpr(AppregateExpr *expr)
	{
		for (int32_t i = (int32_t)expr->exprs.size() - 1; i >= 0; --i)
			CompileExpr(expr->exprs[i]);
		EmitOpCode(OP_ARRAY, expr->tagToken);

		uint8_t pos = (uint8_t)expr->exprs.size();
		Emit(pos);
	}

	void Compiler::CompileDictExpr(DictExpr *expr)
	{
		for (auto i = 0; i < expr->elements.size(); ++i)
		{
			CompileExpr(expr->elements[i].first);
			CompileExpr(expr->elements[i].second);
		}
		EmitOpCode(OP_DICT, expr->tagToken);
		uint8_t pos = static_cast<uint8_t>(expr->elements.size());
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
		auto callee = (CallExpr *)expr->callee;
		CompileExpr(callee->callee, RWState::READ);
		EmitOpCode(OP_CALL, expr->callee->tagToken);
		Emit(0);
		for (const auto &arg : callee->arguments)
			CompileExpr(arg);
		EmitOpCode(OP_CALL, expr->callee->tagToken);
		Emit(static_cast<uint8_t>(callee->arguments.size()));
	}

	void Compiler::CompileThisExpr(ThisExpr *expr)
	{
		auto identExpr = new IdentifierExpr(expr->tagToken, TEXT("this"));
		CompileExpr(identExpr);
	}

	void Compiler::CompileBaseExpr(BaseExpr *expr)
	{
		auto identExpr = new IdentifierExpr(expr->tagToken, TEXT("this"));
		CompileExpr(identExpr);
		EmitConstant(new StrObject(expr->callMember->ToString()), expr->tagToken);
		EmitOpCode(OP_GET_BASE, expr->callMember->tagToken);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state, int8_t paramCount)
	{
		OpCode getOp, setOp;
		auto symbol = mSymbolTable->Resolve(expr->tagToken, expr->literal, paramCount);
		if (symbol.location == SymbolLocation::GLOBAL)
		{
			getOp = OP_GET_GLOBAL;
			setOp = OP_SET_GLOBAL;
		}
		else if (symbol.location == SymbolLocation::LOCAL)
		{
			getOp = OP_GET_LOCAL;
			setOp = OP_SET_LOCAL;
		}
		else if (symbol.location == SymbolLocation::UPVALUE)
		{
			getOp = OP_GET_UPVALUE;
			setOp = OP_SET_UPVALUE;
		}

		if (state == RWState::WRITE)
		{
			if (symbol.privilege == Privilege::MUTABLE)
			{
				EmitOpCode(setOp, expr->tagToken);
				if (symbol.location == SymbolLocation::UPVALUE)
					Emit(symbol.upvalue.index);
				else
					Emit(symbol.index);
			}
			else
				Logger::Error(expr->tagToken, TEXT("{} is a constant,which cannot be assigned!"), expr->ToString());
		}
		else
		{
			EmitOpCode(getOp, expr->tagToken);
			if (symbol.location == SymbolLocation::UPVALUE)
				Emit(symbol.upvalue.index);
			else
				Emit(symbol.index);
		}
	}
	void Compiler::CompileLambdaExpr(LambdaExpr *expr)
	{
		mFunctionList.emplace_back(new FunctionObject());
		mSymbolTable = new SymbolTable(mSymbolTable);

		mSymbolTable->Define(expr->tagToken, Privilege::IMMUTABLE, TEXT(""));

		auto varArg = GetVarArgFromParameterList(expr->parameters);

		CurFunction()->arity = static_cast<uint8_t>(expr->parameters.size());
		CurFunction()->varArg = varArg;

		for (const auto &param : expr->parameters)
		{
			auto varDescExpr = (VarDescExpr *)param;
			if (varDescExpr->name->kind == AstKind::IDENTIFIER)
				mSymbolTable->Define(varDescExpr->tagToken, Privilege::MUTABLE, ((IdentifierExpr *)((VarDescExpr *)param)->name)->literal);
			else if (varDescExpr->name->kind == AstKind::VAR_ARG)
			{
				auto varArg = ((VarArgExpr *)varDescExpr->name);
				if (varArg->argName)
					mSymbolTable->Define(varArg->tagToken, Privilege::MUTABLE, varArg->argName->literal);
			}
		}

		EnterScope();

		CompileScopeStmt(expr->body);

		if (CurChunk().opCodes[CurChunk().opCodes.size() - 2] != OP_RETURN)
			EmitReturn(0, expr->body->stmts.back()->tagToken);

		mSymbolTable = mSymbolTable->enclosing;

		auto function = mFunctionList.back();
		mFunctionList.pop_back();

		EmitClosure(function, expr->tagToken);
	}

	void Compiler::CompileCompoundExpr(CompoundExpr *expr)
	{
		EnterScope();
		for (const auto &s : expr->stmts)
			CompileDecl(s);
		CompileExpr(expr->endExpr);
		ExitScope();
	}

	void Compiler::CompileCallExpr(CallExpr *expr)
	{
		CompileExpr(expr->callee, RWState::READ, static_cast<int8_t>(expr->arguments.size()));
		for (const auto &arg : expr->arguments)
			CompileExpr(arg);
		EmitOpCode(OP_CALL, expr->callee->tagToken);
		Emit(static_cast<uint8_t>(expr->arguments.size()));
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
		if (expr->refExpr->kind == AstKind::INDEX)
		{
			auto refIdxExpr = ((IndexExpr *)expr->refExpr);
			CompileExpr(refIdxExpr->index);
			symbol = mSymbolTable->Resolve(refIdxExpr->ds->tagToken, refIdxExpr->ds->ToString());
			if (symbol.location == SymbolLocation::GLOBAL)
			{
				EmitOpCode(OP_REF_INDEX_GLOBAL, symbol.relatedToken);
				Emit(symbol.index);
			}
			else if (symbol.location == SymbolLocation::LOCAL)
			{
				EmitOpCode(OP_REF_INDEX_LOCAL, symbol.relatedToken);
				Emit(symbol.index);
			}
			else if (symbol.location == SymbolLocation::UPVALUE)
			{
				EmitOpCode(OP_REF_INDEX_UPVALUE, symbol.relatedToken);
				Emit(symbol.upvalue.index);
			}
		}
		else
		{
			symbol = mSymbolTable->Resolve(expr->refExpr->tagToken, expr->refExpr->ToString());
			if (symbol.location == SymbolLocation::GLOBAL)
			{
				EmitOpCode(OP_REF_GLOBAL, symbol.relatedToken);
				Emit(symbol.index);
			}
			else if (symbol.location == SymbolLocation::LOCAL)
			{
				EmitOpCode(OP_REF_LOCAL, symbol.relatedToken);
				Emit(symbol.index);
			}
			else if (symbol.location == SymbolLocation::UPVALUE)
			{
				EmitOpCode(OP_REF_UPVALUE, symbol.relatedToken);
				Emit(symbol.upvalue.index);
			}
		}
	}

	void Compiler::CompileStructExpr(StructExpr *expr)
	{
		for (auto [k, v] : expr->elements)
		{
			CompileExpr(v);
			EmitConstant(new StrObject(k), v->tagToken);
		}
		EmitOpCode(OP_STRUCT, expr->tagToken);
		uint8_t pos = static_cast<uint8_t>(expr->elements.size());
		Emit(pos);
	}

	void Compiler::CompileVarArgExpr(VarArgExpr *expr, const RWState &state)
	{
		if (expr->argName)
			CompileExpr(expr->argName, state);
	}

	void Compiler::CompileFactorialExpr(FactorialExpr *expr, const RWState &state)
	{
		CompileExpr(expr->expr, state);
		EmitOpCode(OP_FACTORIAL, expr->tagToken);
	}

	Symbol Compiler::CompileFunction(FunctionStmt *stmt)
	{
		auto varArg = GetVarArgFromParameterList(stmt->parameters);

		auto functionSymbol = mSymbolTable->Define(stmt->tagToken, Privilege::IMMUTABLE, stmt->name->literal, FunctionSymbolInfo{(int8_t)stmt->parameters.size(), varArg});

		mFunctionList.emplace_back(new FunctionObject(stmt->name->literal));
		mSymbolTable = new SymbolTable(mSymbolTable);

		STD_STRING symbolName = stmt->name->literal;
		if (stmt->functionKind == FunctionKind::CLASS_CLOSURE || stmt->functionKind == FunctionKind::CLASS_CONSTRUCTOR)
			symbolName = TEXT("this");
		mSymbolTable->Define(stmt->tagToken, Privilege::IMMUTABLE, symbolName);

		CurFunction()->arity = static_cast<uint8_t>(stmt->parameters.size());
		CurFunction()->varArg = varArg;

		for (const auto &param : stmt->parameters)
		{
			auto varDescExpr = (VarDescExpr *)param;
			if (varDescExpr->name->kind == AstKind::IDENTIFIER)
				mSymbolTable->Define(varDescExpr->tagToken, Privilege::MUTABLE, ((IdentifierExpr *)((VarDescExpr *)param)->name)->literal);
			else if (varDescExpr->name->kind == AstKind::VAR_ARG)
			{
				auto varArg = ((VarArgExpr *)varDescExpr->name);
				if (varArg->argName)
					mSymbolTable->Define(varArg->argName->tagToken, Privilege::MUTABLE, varArg->argName->literal);
			}
		}

		EnterScope();

		CompileScopeStmt(stmt->body);

		if (stmt->functionKind == FunctionKind::CLASS_CONSTRUCTOR)
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

	uint32_t Compiler::CompileVars(VarStmt *stmt, bool IsInClassOrModuleScope)
	{
		uint32_t varCount = 0;

		auto postfixExprs = StatsPostfixExprs(stmt);

		{
			for (const auto &[k, v] : stmt->variables)
			{
				// destructuring assignment like let [x,y,...args]=....
				if (k->kind == AstKind::ARRAY)
				{
					auto arrayExpr = (ArrayExpr *)k;

					uint64_t resolveAddress = 0;
					OpCode appregateOpCode = OP_APPREGATE_RESOLVE;
					uint64_t appregateOpCodeAddress;

					// compile right value
					{
						CompileExpr(v);

						appregateOpCodeAddress = EmitOpCode((OpCode)0xFF, arrayExpr->tagToken) - static_cast<uint64_t>(1);
						resolveAddress = Emit((OpCode)0xFF);
					}

					int32_t resolveCount = 0;

					if (mSymbolTable->enclosing == nullptr && mSymbolTable->mScopeDepth > 0) // local scope
						std::reverse(arrayExpr->elements.begin(), arrayExpr->elements.end());

					for (int32_t i = 0; i < arrayExpr->elements.size(); ++i)
					{
						Symbol symbol;
						STD_STRING literal;
						Token *token = nullptr;

						if (((VarDescExpr *)arrayExpr->elements[i])->name->kind == AstKind::IDENTIFIER)
						{
							literal = ((IdentifierExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->literal;
							token = ((IdentifierExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->tagToken;
							symbol = mSymbolTable->Define(token, stmt->privilege, literal);
							resolveCount++;
						}
						else if (((VarDescExpr *)arrayExpr->elements[i])->name->kind == AstKind::VAR_ARG)
						{
							// varArg with name like:let [x,y,...args] (means IdentifierExpr* in VarDescExpr* not nullptr)
							if (((VarArgExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->argName)
							{
								literal = ((VarArgExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->argName->literal;
								token = ((VarArgExpr *)((VarDescExpr *)arrayExpr->elements[i])->name)->argName->tagToken;
								symbol = mSymbolTable->Define(token, stmt->privilege, literal);
								resolveCount++;
								appregateOpCode = OP_APPREGATE_RESOLVE_VAR_ARG;
							}
							else // var arg without names like: let [x,y,...]
								continue;
						}

						if (symbol.location == SymbolLocation::GLOBAL)
						{
							EmitOpCode(OP_SET_GLOBAL, symbol.relatedToken);
							Emit(symbol.index);
							EmitOpCode(OP_POP, symbol.relatedToken);
						}
						else if (IsInClassOrModuleScope)
						{
							EmitConstant(new StrObject(literal), token);
						}
						varCount++;
					}

					CurOpCodes()[appregateOpCodeAddress] = appregateOpCode;
					CurOpCodes()[resolveAddress] = resolveCount;

					if (IsInClassOrModuleScope)
					{
						EmitOpCode(OP_RESET, stmt->tagToken);
						Emit(varCount);
					}
				}
				else if (k->kind == AstKind::VAR_DESC)
				{
					CompileExpr(v);

					STD_STRING literal;
					Token *token = nullptr;

					if (((VarDescExpr *)k)->name->kind == AstKind::IDENTIFIER)
					{
						literal = ((IdentifierExpr *)(((VarDescExpr *)k)->name))->literal;
						token = ((IdentifierExpr *)(((VarDescExpr *)k)->name))->tagToken;
					}

					else if (((VarDescExpr *)k)->name->kind == AstKind::VAR_ARG)
					{
						literal = ((VarArgExpr *)((VarDescExpr *)k)->name)->argName->literal;
						token = ((VarArgExpr *)((VarDescExpr *)k)->name)->argName->tagToken;
					}

					auto symbol = mSymbolTable->Define(token, stmt->privilege, literal);
					if (symbol.location == SymbolLocation::GLOBAL)
					{
						EmitOpCode(OP_SET_GLOBAL, symbol.relatedToken);
						Emit(symbol.index);
						EmitOpCode(OP_POP, symbol.relatedToken);
					}
					else if (IsInClassOrModuleScope)
					{
						EmitConstant(new StrObject(literal), token);
					}
					varCount++;
				}
				else
					Logger::Error(k->tagToken, TEXT("Unknown variable:{}"), k->ToString());
			}
		}

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, RWState::READ, false);
		}

		return varCount;
	}

	Symbol Compiler::CompileClass(ClassStmt *stmt)
	{
		auto symbol = mSymbolTable->Define(stmt->tagToken, Privilege::IMMUTABLE, stmt->name);

		mFunctionList.emplace_back(new FunctionObject(stmt->name));
		mSymbolTable = new SymbolTable(mSymbolTable);

		mSymbolTable->Define(stmt->tagToken, Privilege::IMMUTABLE, TEXT(""));

		int8_t varCount = 0;
		int8_t constCount = 0;

		for (const auto &enumStmt : stmt->enumItems)
		{
			CompileEnumDecl(enumStmt);
			EmitConstant(new StrObject(enumStmt->name->literal), enumStmt->tagToken);
			constCount++;
		}

		for (const auto &fnStmt : stmt->fnItems)
		{
			CompileFunction(fnStmt);
			EmitConstant(new StrObject(fnStmt->name->literal), fnStmt->tagToken);
			constCount++;
		}

		for (const auto &varStmt : stmt->varItems)
		{
			if (varStmt->privilege == Privilege::IMMUTABLE)
				constCount += CompileVars(varStmt, true);
		}

		for (const auto &varStmt : stmt->varItems)
		{
			if (varStmt->privilege == Privilege::MUTABLE)
				varCount += CompileVars(varStmt, true);
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
		Emit(static_cast<uint8_t>(stmt->constructors.size()));
		Emit(varCount);
		Emit(constCount);
		Emit(static_cast<uint8_t>(stmt->parentClasses.size()));

		EmitReturn(1, stmt->tagToken);

		mSymbolTable = mSymbolTable->enclosing;

		auto function = mFunctionList.back();
		mFunctionList.pop_back();

		EmitClosure(function, stmt->tagToken);

		return symbol;
	}

	uint64_t Compiler::EmitOpCode(OpCode opCode, const Token *token)
	{
		Emit((uint8_t)opCode);

		CurChunk().opCodeRelatedTokens.emplace_back(token);

		Emit(static_cast<uint8_t>(CurChunk().opCodeRelatedTokens.size() - 1));

		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::Emit(uint8_t opcode)
	{
		CurOpCodes().emplace_back(opcode);
		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::EmitConstant(const Value &value, const Token *token)
	{
		EmitOpCode(OP_CONSTANT, token);
		uint8_t pos = AddConstant(value);
		Emit(pos);
		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::EmitClosure(FunctionObject *function, const Token *token)
	{
		uint8_t pos = AddConstant(function);
		EmitOpCode(OP_CLOSURE, token);
		Emit(pos);
		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::EmitReturn(uint8_t retCount, const Token *token)
	{
		EmitOpCode(OP_RETURN, token);
		Emit(retCount);
		return CurOpCodes().size() - 1;
	}

	uint64_t Compiler::EmitJump(OpCode opcode, const Token *token)
	{
		EmitOpCode(opcode, token);
		Emit(0xFF);
		Emit(0xFF);
		return CurOpCodes().size() - 2;
	}

	void Compiler::EmitLoop(uint16_t opcode, const Token *token)
	{
		EmitOpCode(OP_LOOP, token);
		uint16_t offset = static_cast<uint16_t>(CurOpCodes().size()) - opcode + 2;

		Emit((offset >> 8) & 0xFF);
		Emit(offset & 0xFF);
	}

	void Compiler::PatchJump(uint64_t offset)
	{
		uint16_t jumpOffset = static_cast<uint16_t>(CurOpCodes().size() - offset - 2);
		CurOpCodes()[offset] = (jumpOffset >> 8) & 0xFF;
		CurOpCodes()[offset + 1] = (jumpOffset)&0xFF;
	}

	uint8_t Compiler::AddConstant(const Value &value)
	{
		CurChunk().constants.emplace_back(value);
		return static_cast<uint8_t>(CurChunk().constants.size()) - 1;
	}

	void Compiler::EmitSymbol(const Symbol &symbol)
	{
		if (symbol.location == SymbolLocation::GLOBAL)
		{
			EmitOpCode(OP_SET_GLOBAL, symbol.relatedToken);
			Emit(symbol.index);
			EmitOpCode(OP_POP, symbol.relatedToken);
		}
		else if (symbol.location == SymbolLocation::LOCAL)
		{
			EmitOpCode(OP_SET_LOCAL, symbol.relatedToken);
			Emit(symbol.index);
		}
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
			if (symbol->location == SymbolLocation::LOCAL &&
				symbol->scopeDepth > mSymbolTable->mScopeDepth)
			{
				if (symbol->isCaptured)
					EmitOpCode(OP_CLOSE_UPVALUE, symbol->relatedToken);
				else
					EmitOpCode(OP_POP, symbol->relatedToken);
				symbol->location = SymbolLocation::GLOBAL; // mark as global to avoid second pop
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

	VarArg Compiler::GetVarArgFromParameterList(const std::vector<VarDescExpr *> &parameterList)
	{
		VarArg result{VarArg::NONE};
		if (!parameterList.empty() && parameterList.back()->name->kind == AstKind::VAR_ARG)
		{
			auto varArg = (VarArgExpr *)parameterList.back()->name;
			if (varArg->argName)
				result = VarArg::WITH_NAME;
			else
				result = VarArg::WITHOUT_NAME;
		}
		return result;
	}

	std::vector<Expr *> Compiler::StatsPostfixExprs(AstNode *astNode)
	{
		if (!astNode) // check astnode is nullptr
			return {};

		switch (astNode->kind)
		{
		case AstKind::BREAK:
		case AstKind::CONTINUE:
		case AstKind::LITERAL:
		case AstKind::IDENTIFIER:
			return {};
		case AstKind::ASTSTMTS:
		{
			std::vector<Expr *> result;
			for (const auto &stmt : ((AstStmts *)astNode)->stmts)
			{
				auto stmtResult = StatsPostfixExprs(stmt);
				result.insert(result.end(), stmtResult.begin(), stmtResult.end());
			}
			return result;
		}
		case AstKind::RETURN:
			return StatsPostfixExprs(((ReturnStmt *)astNode)->expr);
		case AstKind::EXPR:
			return StatsPostfixExprs(((ExprStmt *)astNode)->expr);
		case AstKind::VAR:
		{
			std::vector<Expr *> result;
			for (const auto &[k, v] : ((VarStmt *)astNode)->variables)
			{
				auto varResult = StatsPostfixExprs(v);
				result.insert(result.end(), varResult.begin(), varResult.end());
			}
			return result;
		}
		case AstKind::SCOPE:
		{
			std::vector<Expr *> result;
			for (const auto &stmt : ((ScopeStmt *)astNode)->stmts)
			{
				auto stmtResult = StatsPostfixExprs(stmt);
				result.insert(result.end(), stmtResult.begin(), stmtResult.end());
			}
			return result;
		}
		case AstKind::IF:
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
		case AstKind::WHILE:
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
		case AstKind::ENUM:
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
		case AstKind::FUNCTION:
		{
			std::vector<Expr *> result;
			auto bodyResult = StatsPostfixExprs(((FunctionStmt *)astNode)->body);
			result.insert(result.end(), bodyResult.begin(), bodyResult.end());
			return result;
		}
		case AstKind::CLASS:
		{
			std::vector<Expr *> result;

			for (const auto &varStmt : ((ClassStmt *)astNode)->varItems)
			{
				auto letStmtResult = StatsPostfixExprs(varStmt);
				result.insert(result.end(), letStmtResult.begin(), letStmtResult.end());
			}

			for (const auto &fnStmt : ((ClassStmt *)astNode)->fnItems)
			{
				auto fnStmtResult = StatsPostfixExprs(fnStmt);
				result.insert(result.end(), fnStmtResult.begin(), fnStmtResult.end());
			}
			return result;
		}
		case AstKind::GROUP:
			return StatsPostfixExprs(((GroupExpr *)astNode)->expr);
		case AstKind::ARRAY:
		{
			std::vector<Expr *> result;
			for (const auto &e : ((ArrayExpr *)astNode)->elements)
			{
				auto eResult = StatsPostfixExprs(e);
				result.insert(result.end(), eResult.begin(), eResult.end());
			}
			return result;
		}
		case AstKind::DICT:
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
		case AstKind::INDEX:
		{
			std::vector<Expr *> result;
			auto dsResult = StatsPostfixExprs(((IndexExpr *)astNode)->ds);
			result.insert(result.end(), dsResult.begin(), dsResult.end());
			auto indexResult = StatsPostfixExprs(((IndexExpr *)astNode)->index);
			result.insert(result.end(), indexResult.begin(), indexResult.end());
			return result;
		}
		case AstKind::PREFIX:
			return StatsPostfixExprs(((PrefixExpr *)astNode)->right);
		case AstKind::INFIX:
		{
			std::vector<Expr *> result;
			auto leftResult = StatsPostfixExprs(((InfixExpr *)astNode)->left);
			auto rightResult = StatsPostfixExprs(((InfixExpr *)astNode)->right);

			result.insert(result.end(), leftResult.begin(), leftResult.end());
			result.insert(result.end(), rightResult.begin(), rightResult.end());
			return result;
		}
		case AstKind::POSTFIX:
		{
			std::vector<Expr *> result;
			auto leftResult = StatsPostfixExprs(((PostfixExpr *)astNode)->left);
			result.insert(result.end(), leftResult.begin(), leftResult.end());
			result.emplace_back((PostfixExpr *)astNode);
			return result;
		}
		case AstKind::CONDITION:
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
		case AstKind::LAMBDA:
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
		case AstKind::CALL:
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
		case AstKind::REF:
			return StatsPostfixExprs(((RefExpr *)astNode)->refExpr);
		default:
			return {};
		}
		return {};
	}
}