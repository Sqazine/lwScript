#include "Compiler.h"
#include "Utils.h"
#include "Library.h"
#include "Config.h"
namespace lws
{
	Compiler::Compiler()
		: mRootFrame(new Frame())
	{
	}
	Compiler::~Compiler()
	{
		if (mRootFrame)
		{
			delete mRootFrame;
			mRootFrame = nullptr;
		}
	}

	Frame *Compiler::Compile(Stmt *stmt)
	{
		CompileAstStmts((AstStmts *)stmt, mRootFrame);
		return mRootFrame;
	}

	void Compiler::ResetStatus()
	{
		mRootFrame->Clear();
	}

	void Compiler::CompileAstStmts(AstStmts *stmt, Frame *frame)
	{
		for (const auto &s : stmt->stmts)
			CompileStmt(s, frame);
	}

	void Compiler::CompileStmt(Stmt *stmt, Frame *frame, uint64_t breakStmtAddressOffset, uint64_t continueStmtAddressOffset)
	{
		switch (stmt->Type())
		{
		case AST_RETURN:
			CompileReturnStmt((ReturnStmt *)stmt, frame);
			break;
		case AST_EXPR:
			CompileExprStmt((ExprStmt *)stmt, frame);
			break;
		case AST_LET:
			CompileLetStmt((LetStmt *)stmt, frame);
			break;
		case AST_CONST:
			CompileConstStmt((ConstStmt *)stmt, frame);
			break;
		case AST_SCOPE:
			CompileScopeStmt((ScopeStmt *)stmt, frame, breakStmtAddressOffset, continueStmtAddressOffset);
			break;
		case AST_IF:
			CompileIfStmt((IfStmt *)stmt, frame, breakStmtAddressOffset, continueStmtAddressOffset);
			break;
		case AST_WHILE:
			CompileWhileStmt((WhileStmt *)stmt, frame);
			break;
		case AST_BREAK:
			CompileBreakStmt(breakStmtAddressOffset, frame);
			break;
		case AST_CONTINUE:
			CompileContinueStmt(continueStmtAddressOffset, frame);
			break;
		case AST_ENUM:
			CompileEnumStmt((EnumStmt *)stmt, frame);
			break;
		case AST_FUNCTION:
			CompileFunctionStmt((FunctionStmt *)stmt, frame);
			break;
		case AST_FIELD:
			CompileFieldStmt((FieldStmt *)stmt, frame);
			break;
		default:
			break;
		}
	}
	void Compiler::CompileReturnStmt(ReturnStmt *stmt, Frame *frame)
	{
		auto postfixExprs = StatsPostfixExprs(stmt->expr);
		if (stmt->expr)
		{
			CompileExpr(stmt->expr, frame);
			frame->AddOpCode(OP_NEW_INT);
			size_t offset = frame->AddIntNum(1);
			frame->AddOpCode(offset);
		}
		else
		{
			frame->AddOpCode(OP_NEW_INT);
			size_t offset = frame->AddIntNum(0);
			frame->AddOpCode(offset);
		}
		frame->AddOpCode(OP_RETURN);

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, frame, false);
		}
	}

	void Compiler::CompileExprStmt(ExprStmt *stmt, Frame *frame)
	{
		auto postfixExprs = StatsPostfixExprs(stmt->expr);

		CompileExpr(stmt->expr, frame);

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, frame, false);
		}
	}

	void Compiler::CompileLetStmt(LetStmt *stmt, Frame *frame)
	{
		auto postfixExprs = StatsPostfixExprs(stmt);

		for (const auto &[k, v] : stmt->variables)
		{
			CompileExpr(v.value, frame);
			CompileExpr(k, frame, VAR_INIT);
		}

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, frame, false);
		}
	}

	void Compiler::CompileConstStmt(ConstStmt *stmt, Frame *frame)
	{
		auto postfixExprs = StatsPostfixExprs(stmt);

		for (const auto &[k, v] : stmt->consts)
		{
			CompileExpr(v.value, frame);
			CompileExpr(k, frame, CONST_INIT);
		}

		if (!postfixExprs.empty())
		{
			for (const auto &postfixExpr : postfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, frame, false);
		}
	}

	void Compiler::CompileScopeStmt(ScopeStmt *stmt, Frame *frame, uint64_t breakStmtAddressOffset, uint64_t continueStmtAddressOffset)
	{
		frame->AddOpCode(OP_ENTER_SCOPE);

		for (const auto &s : stmt->stmts)
			CompileStmt(s, frame, breakStmtAddressOffset, continueStmtAddressOffset);

		frame->AddOpCode(OP_EXIT_SCOPE);
	}

	void Compiler::CompileIfStmt(IfStmt *stmt, Frame *frame, uint64_t breakStmtAddressOffset, uint64_t continueStmtAddressOffset)
	{
		auto conditionPostfixExprs = StatsPostfixExprs(stmt->condition);

		CompileExpr(stmt->condition, frame);

		if (!conditionPostfixExprs.empty())
		{
			for (const auto &postfixExpr : conditionPostfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, frame, false);
		}

		frame->AddOpCode(OP_JUMP_IF_FALSE);
		uint64_t jmpIfFalseOffset = frame->AddIntNum(0);
		frame->AddOpCode(jmpIfFalseOffset);

		CompileStmt(stmt->thenBranch, frame, jmpIfFalseOffset, continueStmtAddressOffset);

		frame->AddOpCode(OP_JUMP);
		uint64_t jmpOffset = frame->AddIntNum(0);
		frame->AddOpCode(jmpOffset);

		frame->mIntNums[jmpIfFalseOffset] = frame->mCodes.size() - 1;

		if (stmt->elseBranch)
			CompileStmt(stmt->elseBranch, frame, breakStmtAddressOffset, continueStmtAddressOffset);

		frame->mIntNums[jmpOffset] = frame->mCodes.size() - 1;
	}
	void Compiler::CompileWhileStmt(WhileStmt *stmt, Frame *frame)
	{
		uint64_t jmpAddress = frame->mCodes.size() - 1;

		auto conditionPostfixExprs = StatsPostfixExprs(stmt->condition);

		CompileExpr(stmt->condition, frame);

		if (!conditionPostfixExprs.empty())
		{
			for (const auto &postfixExpr : conditionPostfixExprs)
				CompilePostfixExpr((PostfixExpr *)postfixExpr, frame, false);
		}

		frame->AddOpCode(OP_JUMP_IF_FALSE);
		uint64_t jmpIfFalseOffset = frame->AddIntNum(0);
		frame->AddOpCode(jmpIfFalseOffset);

		uint64_t offset = frame->AddIntNum(jmpAddress);
		if (!stmt->increment)
			CompileStmt(stmt->body, frame, jmpIfFalseOffset, offset);
		else
		{
			uint64_t incrementPartOffset = frame->AddIntNum(0);
			CompileStmt(stmt->body, frame, jmpIfFalseOffset, incrementPartOffset);
			frame->mIntNums[incrementPartOffset] = frame->mCodes.size() - 1;
			CompileStmt(stmt->increment, frame);
		}

		frame->AddOpCode(OP_JUMP);
		frame->AddOpCode(offset);

		frame->mIntNums[jmpIfFalseOffset] = frame->mCodes.size() - 1;
	}

	void Compiler::CompileBreakStmt(uint64_t addressOffset, Frame *frame)
	{
		CompileBreakAndContinueStmt(addressOffset, frame);
	}

	void Compiler::CompileContinueStmt(uint64_t addressOffset, Frame *frame)
	{
		CompileBreakAndContinueStmt(addressOffset, frame);
	}

	void Compiler::CompileEnumStmt(EnumStmt *enumStmt, Frame *frame)
	{
		Frame *enumFrame = new Frame(frame);

		enumFrame->AddOpCode(OP_ENTER_SCOPE);

		for (const auto &[k, v] : enumStmt->enumItems)
		{
			CompileExpr(v, enumFrame);
			CompileExpr(k, enumFrame, CONST_INIT);
		}

		enumFrame->AddOpCode(OP_NEW_FIELD);
		uint64_t offset = enumFrame->AddString(enumStmt->enumName->literal);
		enumFrame->AddOpCode(offset);

		enumFrame->AddOpCode(OP_SAVE_TO_GLOBAL);
		offset = enumFrame->AddString(enumStmt->enumName->literal);
		enumFrame->AddOpCode(offset);

		frame->AddEnumFrame(enumStmt->enumName->literal, enumFrame);
	}

	void Compiler::CompileFunctionStmt(FunctionStmt *stmt, Frame *frame)
	{
		Frame *functionFrame = new Frame(frame);

		functionFrame->AddOpCode(OP_ENTER_SCOPE);

		for (int64_t i = stmt->parameters.size() - 1; i >= 0; --i)
			CompileIdentifierExpr(stmt->parameters[i], functionFrame, VAR_INIT);

		for (const auto &s : stmt->body->stmts)
			CompileStmt(s, functionFrame);

		functionFrame->AddOpCode(OP_EXIT_SCOPE);

		frame->AddFunctionFrame(stmt->name->literal + functionNameAndArgumentConnector + std::to_wstring(stmt->parameters.size()), functionFrame);
	}

	void Compiler::CompileLambdaExpr(LambdaExpr *stmt, Frame *frame)
	{
		Frame *lambdaFrame = new Frame(frame);

		lambdaFrame->AddOpCode(OP_ENTER_SCOPE);

		for (int64_t i = stmt->parameters.size() - 1; i >= 0; --i)
			CompileIdentifierExpr(stmt->parameters[i], lambdaFrame, VAR_INIT);

		for (const auto &s : stmt->body->stmts)
			CompileStmt(s, lambdaFrame);

		lambdaFrame->AddOpCode(OP_EXIT_SCOPE);

		frame->AddOpCode(OP_NEW_LAMBDA);
		size_t offset = frame->AddIntNum(frame->AddLambdaFrame(lambdaFrame));
		frame->AddOpCode(offset);
	}

	void Compiler::CompileFieldStmt(FieldStmt *stmt, Frame *frame)
	{
		Frame *fieldFrame = new Frame(frame);

		fieldFrame->AddOpCode(OP_ENTER_SCOPE);

		for (const auto &containedField : stmt->containedFields)
		{
			CompileIdentifierExpr(containedField, fieldFrame, VAR_READ);
			IdentifierExpr *instanceIdetExpr = new IdentifierExpr(containedFieldPrefixID + containedField->literal);
			CompileIdentifierExpr(instanceIdetExpr, fieldFrame, VAR_INIT);
		}

		for (auto &letStmt : stmt->letStmts)
		{
			for (auto &variable : letStmt->variables)
				if (variable.second.value->Type() == AST_LAMBDA)
					((LambdaExpr *)variable.second.value)->parameters.emplace_back(new IdentifierExpr(L"this")); //add 'this' parameter for field lambda function

			CompileLetStmt(letStmt, fieldFrame);
		}

		for (auto &constStmt : stmt->constStmts)
		{
			for (auto &variable : constStmt->consts)
				if (variable.second.value->Type() == AST_LAMBDA)
					((LambdaExpr *)variable.second.value)->parameters.emplace_back(new IdentifierExpr(L"this")); //add 'this' parameter for field lambda function

			CompileConstStmt(constStmt, fieldFrame);
		}

		for (const auto &functionStmt : stmt->fnStmts)
		{
			functionStmt->parameters.emplace_back(new IdentifierExpr(L"this")); //regisiter field instance to function

			CompileFunctionStmt(functionStmt, fieldFrame);
		}

		fieldFrame->AddOpCode(OP_NEW_FIELD);
		uint64_t offset = fieldFrame->AddString(stmt->name);
		fieldFrame->AddOpCode(offset);

		fieldFrame->AddOpCode(OP_NEW_INT);
		offset = fieldFrame->AddIntNum(1);
		fieldFrame->AddOpCode(offset);

		fieldFrame->AddOpCode(OP_RETURN);

		frame->AddFieldFrame(stmt->name, fieldFrame);
	}

	void Compiler::CompileExpr(Expr *expr, Frame *frame, ObjectState state)
	{
		switch (expr->Type())
		{
		case AST_REAL:
			CompileRealNumExpr((RealNumExpr *)expr, frame);
			break;
		case AST_INT:
			CompileIntNumExpr((IntNumExpr *)expr, frame);
			break;
		case AST_STR:
			CompileStrExpr((StrExpr *)expr, frame);
			break;
		case AST_BOOL:
			CompileBoolExpr((BoolExpr *)expr, frame);
			break;
		case AST_NULL:
			CompileNullExpr((NullExpr *)expr, frame);
			break;
		case AST_IDENTIFIER:
			CompileIdentifierExpr((IdentifierExpr *)expr, frame, state);
			break;
		case AST_GROUP:
			CompileGroupExpr((GroupExpr *)expr, frame);
			break;
		case AST_ARRAY:
			CompileArrayExpr((ArrayExpr *)expr, frame);
			break;
		case AST_TABLE:
			CompileTableExpr((TableExpr *)expr, frame);
			break;
		case AST_INDEX:
			CompileIndexExpr((IndexExpr *)expr, frame, state);
			break;
		case AST_PREFIX:
			CompilePrefixExpr((PrefixExpr *)expr, frame);
			break;
		case AST_INFIX:
			CompileInfixExpr((InfixExpr *)expr, frame);
			break;
		case AST_POSTFIX:
			CompilePostfixExpr((PostfixExpr *)expr, frame);
			break;
		case AST_CONDITION:
			CompileConditionExpr((ConditionExpr *)expr, frame);
			break;
		case AST_LAMBDA:
			CompileLambdaExpr((LambdaExpr *)expr, frame);
			break;
		case AST_FUNCTION_CALL:
			CompileFunctionCallExpr((FunctionCallExpr *)expr, frame);
			break;
		case AST_FIELD_CALL:
			CompileFieldCallExpr((FieldCallExpr *)expr, frame, state);
			break;
		case AST_REF:
			CompileRefExpr((RefExpr *)expr, frame);
			break;
		default:
			break;
		}
	}

	void Compiler::CompileIntNumExpr(IntNumExpr *expr, Frame *frame)
	{
		frame->AddOpCode(OP_NEW_INT);
		size_t offset = frame->AddIntNum(expr->value);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileRealNumExpr(RealNumExpr *expr, Frame *frame)
	{
		frame->AddOpCode(OP_NEW_REAL);
		size_t offset = frame->AddRealNum(expr->value);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileStrExpr(StrExpr *expr, Frame *frame)
	{
		frame->AddOpCode(OP_NEW_STR);
		size_t offset = frame->AddString(expr->value);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileBoolExpr(BoolExpr *expr, Frame *frame)
	{
		if (expr->value)
			frame->AddOpCode(OP_NEW_TRUE);
		else
			frame->AddOpCode(OP_NEW_FALSE);
	}

	void Compiler::CompileNullExpr(NullExpr *expr, Frame *frame)
	{
		frame->AddOpCode(OP_NEW_NULL);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, Frame *frame, ObjectState state)
	{
		if (state == VAR_READ)
			frame->AddOpCode(OP_GET_VAR);
		else if (state == VAR_WRITE)
			frame->AddOpCode(OP_SET_VAR);
		else if (state == VAR_INIT)
			frame->AddOpCode(OP_NEW_VAR);
		else if (state == CONST_INIT)
			frame->AddOpCode(OP_NEW_CONST);
		else if (state == FIELD_MEMBER_READ)
			frame->AddOpCode(OP_GET_FIELD_VAR);
		else if (state == FIELD_MEMBER_WRITE)
			frame->AddOpCode(OP_SET_FIELD_VAR);
		else if (state == FUNCTION_READ)
			frame->AddOpCode(OP_GET_FUNCTION);
		else if (state == FIELD_FUNCTION_READ)
			frame->AddOpCode(OP_GET_FIELD_FUNCTION);

		uint64_t offset = frame->AddString(expr->literal);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileGroupExpr(GroupExpr *expr, Frame *frame)
	{
		CompileExpr(expr->expr, frame);
	}

	void Compiler::CompileArrayExpr(ArrayExpr *expr, Frame *frame)
	{
		for (const auto &e : expr->elements)
			CompileExpr(e, frame);

		frame->AddOpCode(OP_NEW_ARRAY);
		size_t offset = frame->AddIntNum((int64_t)expr->elements.size());
		frame->AddOpCode(offset);
	}

	void Compiler::CompileTableExpr(TableExpr *expr, Frame *frame)
	{
		for (const auto &[k, v] : expr->elements)
		{
			CompileExpr(v, frame);
			CompileExpr(k, frame);
		}
		frame->AddOpCode(OP_NEW_TABLE);
		uint64_t offset = frame->AddIntNum((int64_t)expr->elements.size());
		frame->AddOpCode(offset);
	}

	void Compiler::CompileIndexExpr(IndexExpr *expr, Frame *frame, ObjectState state)
	{
		CompileExpr(expr->ds, frame);
		CompileExpr(expr->index, frame);
		if (state == VAR_READ)
			frame->AddOpCode(OP_GET_INDEX_VAR);
		else if (state == VAR_WRITE)
			frame->AddOpCode(OP_SET_INDEX_VAR);
	}

	void Compiler::CompilePrefixExpr(PrefixExpr *expr, Frame *frame)
	{
		CompileExpr(expr->right, frame);
		if (expr->op == L"-")
			frame->AddOpCode(OP_NEG);
		else if (expr->op == L"~")
			frame->AddOpCode(OP_BIT_NOT);
		else if (expr->op == L"!")
			frame->AddOpCode(OP_NOT);
		else if (expr->op == L"++")
			frame->AddOpCode(OP_SELF_INCREMENT);
		else if (expr->op == L"--")
			frame->AddOpCode(OP_SELF_DECREMENT);
	}

	void Compiler::CompileInfixExpr(InfixExpr *expr, Frame *frame)
	{
		if (expr->op == L"=")
		{
			CompileExpr(expr->right, frame);
			if (expr->right->Type() == AST_INFIX && ((InfixExpr *)expr->right)->op == L"=") //continuous assignment such as a=b=c;
				CompileExpr(((InfixExpr *)expr->right)->left, frame);

			CompileExpr(expr->left, frame, VAR_WRITE);
		}
		else
		{
			CompileExpr(expr->right, frame);
			CompileExpr(expr->left, frame);

			if (expr->op == L"+")
				frame->AddOpCode(OP_ADD);
			else if (expr->op == L"-")
				frame->AddOpCode(OP_SUB);
			else if (expr->op == L"*")
				frame->AddOpCode(OP_MUL);
			else if (expr->op == L"/")
				frame->AddOpCode(OP_DIV);
			else if (expr->op == L"%")
				frame->AddOpCode(OP_MOD);
			else if (expr->op == L"&")
				frame->AddOpCode(OP_BIT_AND);
			else if (expr->op == L"|")
				frame->AddOpCode(OP_BIT_OR);
			else if (expr->op == L"^")
				frame->AddOpCode(OP_BIT_XOR);
			else if (expr->op == L"&&")
				frame->AddOpCode(OP_AND);
			else if (expr->op == L"||")
				frame->AddOpCode(OP_OR);
			else if (expr->op == L">")
				frame->AddOpCode(OP_GREATER);
			else if (expr->op == L"<")
				frame->AddOpCode(OP_LESS);
			else if (expr->op == L">=")
			{
				frame->AddOpCode(OP_LESS);
				frame->AddOpCode(OP_NOT);
			}
			else if (expr->op == L"<=")
			{
				frame->AddOpCode(OP_GREATER);
				frame->AddOpCode(OP_NOT);
			}
			else if (expr->op == L"==")
				frame->AddOpCode(OP_EQUAL);
			else if (expr->op == L"!=")
			{
				frame->AddOpCode(OP_EQUAL);
				frame->AddOpCode(OP_NOT);
			}
			else if (expr->op == L"<<")
				frame->AddOpCode(OP_BIT_LEFT_SHIFT);
			else if (expr->op == L">>")
				frame->AddOpCode(OP_BIT_RIGHT_SHIFT);
			else if (expr->op == L"+=")
			{
				frame->AddOpCode(OP_ADD);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else if (expr->op == L"-=")
			{
				frame->AddOpCode(OP_SUB);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else if (expr->op == L"*=")
			{
				frame->AddOpCode(OP_MUL);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else if (expr->op == L"/=")
			{
				frame->AddOpCode(OP_DIV);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else if (expr->op == L"&=")
			{
				frame->AddOpCode(OP_BIT_AND);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else if (expr->op == L"|=")
			{
				frame->AddOpCode(OP_BIT_OR);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else if (expr->op == L"^=")
			{
				frame->AddOpCode(OP_BIT_XOR);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else if (expr->op == L"<<=")
			{
				frame->AddOpCode(OP_BIT_LEFT_SHIFT);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else if (expr->op == L">>=")
			{
				frame->AddOpCode(OP_BIT_RIGHT_SHIFT);
				CompileExpr(expr->left, frame, VAR_WRITE);
			}
			else
				Assert(L"Unknown binary op:" + expr->op);
		}
	}

	void Compiler::CompilePostfixExpr(PostfixExpr *expr, Frame *frame, bool isDelayCompile)
	{
		CompileExpr(expr->left, frame);

		//factorial don't need to delay compile
		if (expr->op == L"!")
			frame->AddOpCode(OP_FACTORIAL);
		else if (!isDelayCompile)
		{
			if (expr->op == L"++")
				frame->AddOpCode(OP_SELF_INCREMENT);
			else if (expr->op == L"--")
				frame->AddOpCode(OP_SELF_DECREMENT);
		}
	}

	void Compiler::CompileRefExpr(RefExpr *expr, Frame *frame, ReferenceType type)
	{
		if (type == ReferenceType::VARIABLE)
		{
			if (expr->refExpr->Type() == AST_IDENTIFIER)
			{
				frame->AddOpCode(OP_REF_VARIABLE);
				size_t offset = frame->AddString(((IdentifierExpr *)expr->refExpr)->literal);
				frame->AddOpCode(offset);
			}
			else if (expr->refExpr->Type() == AST_INDEX)
			{
				CompileExpr(((IndexExpr *)expr->refExpr)->index, frame);
				if (((IndexExpr *)expr->refExpr)->ds->Type() != AST_IDENTIFIER)
					Assert(L"Invalid reference object,only left value can be referenced.");
				frame->AddOpCode(OP_REF_INDEX);
				size_t offset = frame->AddString(((IdentifierExpr *)(((IndexExpr *)expr->refExpr)->ds))->literal);
				frame->AddOpCode(offset);
			}
		}
		else
		{
			CompileExpr(expr->refExpr, frame);
			frame->AddOpCode(OP_REF_OBJECT);
		}
	}

	void Compiler::CompileConditionExpr(ConditionExpr *expr, Frame *frame)
	{
		CompileExpr(expr->falseBranch, frame);
		CompileExpr(expr->trueBranch, frame);
		CompileExpr(expr->condition, frame);
		frame->AddOpCode(OP_CONDITION);
	}

	void Compiler::CompileFunctionCallExpr(FunctionCallExpr *expr, Frame *frame)
	{
		for (const auto &arg : expr->arguments)
			CompileExpr(arg, frame);

		//extra args such as 'this' or the field contained field instance
		int64_t extraArgCount = 0;
		if (expr->name->Type() == AST_FIELD_CALL)
		{
			CompileRefExpr(new RefExpr(((FieldCallExpr *)expr->name)->callee), frame, ReferenceType::OBJECT);
			extraArgCount++;
		}
		//argument count
		frame->AddOpCode(OP_NEW_INT);
		uint64_t offset = frame->AddIntNum((int64_t)expr->arguments.size() + extraArgCount);
		frame->AddOpCode(offset);

		if (expr->name->Type() == AST_IDENTIFIER && !LibraryManager::HasNativeFunction(((IdentifierExpr *)expr->name)->literal))
		{
			IdentifierExpr *tmpIden = new IdentifierExpr(((IdentifierExpr *)expr->name)->literal + functionNameAndArgumentConnector + std::to_wstring(expr->arguments.size()));
			CompileExpr(tmpIden, frame, FUNCTION_READ);
		}
		else if (expr->name->Type() == AST_FIELD_CALL)
		{
			FieldCallExpr *fieldCallExpr = (FieldCallExpr *)expr->name;

			CompileExpr(fieldCallExpr->callee, frame);

			if (fieldCallExpr->callMember->Type() == AST_FIELD_CALL) //continuous field call such as a.b.c;
				CompileExpr(((FieldCallExpr *)fieldCallExpr->callMember)->callee, frame, FIELD_MEMBER_READ);

			IdentifierExpr *tmpIden = new IdentifierExpr(((IdentifierExpr *)fieldCallExpr->callMember)->literal + functionNameAndArgumentConnector + std::to_wstring(expr->arguments.size() + 1)); //+1 for adding 'this' to parameter count
			CompileExpr(tmpIden, frame, FIELD_FUNCTION_READ);
		}
		else
			CompileExpr(expr->name, frame, FUNCTION_READ);

		frame->AddOpCode(OP_FUNCTION_CALL);
	}

	void Compiler::CompileFieldCallExpr(FieldCallExpr *expr, Frame *frame, ObjectState state)
	{
		CompileExpr(expr->callee, frame);

		if (expr->callMember->Type() == AST_FIELD_CALL) //continuous field call such as a.b.c;
			CompileExpr(((FieldCallExpr *)expr->callMember)->callee, frame, FIELD_MEMBER_READ);

		if (state == VAR_READ)
			CompileExpr(expr->callMember, frame, FIELD_MEMBER_READ);
		else if (state == VAR_WRITE)
			CompileExpr(expr->callMember, frame, FIELD_MEMBER_WRITE);
	}

	void Compiler::CompileBreakAndContinueStmt(uint64_t addressOffset, Frame *frame)
	{
		frame->AddOpCode(OP_JUMP);
		frame->AddOpCode(addressOffset);
	}

	std::vector<Expr *> Compiler::StatsPostfixExprs(AstNode *astNode)
	{
		if(!astNode)//check astnode is nullptr
			return {};

		switch (astNode->Type())
		{
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
		case AST_BREAK:
			return {};
		case AST_CONTINUE:
			return {};
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
		case AST_FIELD:
		{
			std::vector<Expr *> result;

			for (const auto &letStmt : ((FieldStmt *)astNode)->letStmts)
			{
				auto letStmtResult = StatsPostfixExprs(letStmt);
				result.insert(result.end(), letStmtResult.begin(), letStmtResult.end());
			}

			for (const auto &constStmt : ((FieldStmt *)astNode)->constStmts)
			{
				auto constStmtResult = StatsPostfixExprs(constStmt);
				result.insert(result.end(), constStmtResult.begin(), constStmtResult.end());
			}

			for (const auto &fnStmt : ((FieldStmt *)astNode)->fnStmts)
			{
				auto fnStmtResult = StatsPostfixExprs(fnStmt);
				result.insert(result.end(), fnStmtResult.begin(), fnStmtResult.end());
			}
			return result;
		}
		case AST_REAL:
			return {};
		case AST_INT:
			return {};
		case AST_STR:
			return {};
		case AST_BOOL:
			return {};
		case AST_NULL:
			return {};
		case AST_IDENTIFIER:
			return {};
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
		case AST_FUNCTION_CALL:
		{
			std::vector<Expr *> result;
			auto nameResult = StatsPostfixExprs(((FunctionCallExpr *)astNode)->name);
			result.insert(result.end(), nameResult.begin(), nameResult.end());
			for (const auto &argument : ((FunctionCallExpr *)astNode)->arguments)
			{
				auto argumentResult = StatsPostfixExprs(argument);
				result.insert(result.end(), argumentResult.begin(), argumentResult.end());
			}
			return result;
		}
		case AST_FIELD_CALL:
		{
			std::vector<Expr *> result;
			auto calleeResult = StatsPostfixExprs(((FieldCallExpr *)astNode)->callee);
			result.insert(result.end(), calleeResult.begin(), calleeResult.end());
			auto callMemberResult = StatsPostfixExprs(((FieldCallExpr *)astNode)->callMember);
			result.insert(result.end(), callMemberResult.begin(), callMemberResult.end());
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