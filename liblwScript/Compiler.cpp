#include "Compiler.h"
#include "Utils.h"
#include "Library.h"
#include "Config.h"
namespace lws
{
	Compiler::Compiler()
		: m_RootFrame(new Frame())
	{
	}
	Compiler::~Compiler()
	{
		if (m_RootFrame)
		{
			delete m_RootFrame;
			m_RootFrame = nullptr;
		}
	}

	Frame* Compiler::Compile(Stmt* stmt)
	{
		CompileAstStmts((AstStmts*)stmt, m_RootFrame);
		return m_RootFrame;
	}

	void Compiler::ResetStatus()
	{
		m_RootFrame->Clear();
	}

	void Compiler::CompileAstStmts(AstStmts* stmt, Frame* frame)
	{
		for (const auto& s : stmt->stmts)
			CompileStmt(s, frame);
	}

	void Compiler::CompileStmt(Stmt* stmt, Frame* frame)
	{
		switch (stmt->Type())
		{
		case AST_RETURN:
			CompileReturnStmt((ReturnStmt*)stmt, frame);
			break;
		case AST_EXPR:
			CompileExprStmt((ExprStmt*)stmt, frame);
			break;
		case AST_LET:
			CompileLetStmt((LetStmt*)stmt, frame);
			break;
		case AST_SCOPE:
			CompileScopeStmt((ScopeStmt*)stmt, frame);
			break;
		case AST_IF:
			CompileIfStmt((IfStmt*)stmt, frame);
			break;
		case AST_WHILE:
			CompileWhileStmt((WhileStmt*)stmt, frame);
			break;
		case AST_FUNCTION:
			CompileFunctionStmt((FunctionStmt*)stmt, frame);
			break;
		case AST_FIELD:
			CompileFieldStmt((FieldStmt*)stmt, frame);
			break;
		default:
			break;
		}
	}
	void Compiler::CompileReturnStmt(ReturnStmt* stmt, Frame* frame)
	{
		if (stmt->expr)
			CompileExpr(stmt->expr, frame);

		frame->AddOpCode(OP_RETURN);
	}

	void Compiler::CompileExprStmt(ExprStmt* stmt, Frame* frame)
	{
		CompileExpr(stmt->expr, frame);
	}

	void Compiler::CompileLetStmt(LetStmt* stmt, Frame* frame)
	{
		for (auto [key, value] : stmt->variables)
		{
			CompileExpr(value, frame);
			CompileExpr(key, frame, INIT);
		}
	}

	void Compiler::CompileScopeStmt(ScopeStmt* stmt, Frame* frame)
	{
		frame->AddOpCode(OP_ENTER_SCOPE);

		for (const auto& s : stmt->stmts)
			CompileStmt(s, frame);

		frame->AddOpCode(OP_EXIT_SCOPE);
	}

	void Compiler::CompileIfStmt(IfStmt* stmt, Frame* frame)
	{
		CompileExpr(stmt->condition, frame);

		frame->AddOpCode(OP_JUMP_IF_FALSE);
		uint64_t jmpIfFalseOffset = frame->AddIntNum(0);
		frame->AddOpCode(jmpIfFalseOffset);

		CompileStmt(stmt->thenBranch, frame);

		frame->AddOpCode(OP_JUMP);
		uint64_t jmpOffset = frame->AddIntNum(0);
		frame->AddOpCode(jmpOffset);

		frame->m_IntNums[jmpIfFalseOffset] = frame->GetOpCodeSize() - 1;

		if (stmt->elseBranch)
			CompileStmt(stmt->elseBranch, frame);

		frame->m_IntNums[jmpOffset] = frame->GetOpCodeSize() - 1;
	}
	void Compiler::CompileWhileStmt(WhileStmt* stmt, Frame* frame)
	{
		uint64_t jmpAddress = frame->GetOpCodeSize() - 1;
		CompileExpr(stmt->condition, frame);

		frame->AddOpCode(OP_JUMP_IF_FALSE);
		uint64_t jmpIfFalseOffset = frame->AddIntNum(0);
		frame->AddOpCode(jmpIfFalseOffset);

		CompileStmt(stmt->body, frame);

		frame->AddOpCode(OP_JUMP);
		uint64_t offset = frame->AddIntNum(jmpAddress);
		frame->AddOpCode(offset);

		frame->m_IntNums[jmpIfFalseOffset] = frame->GetOpCodeSize() - 1;
	}

	void Compiler::CompileFunctionStmt(FunctionStmt* stmt, Frame* frame)
	{
		Frame* functionFrame = new Frame(frame);

		functionFrame->AddOpCode(OP_ENTER_SCOPE);

		for (int64_t i = stmt->parameters.size() - 1; i >= 0; --i)
			CompileIdentifierExpr(stmt->parameters[i], functionFrame, INIT);

		for (const auto& s : stmt->body->stmts)
			CompileStmt(s, functionFrame);

		functionFrame->AddOpCode(OP_EXIT_SCOPE);

		frame->AddFunctionFrame(stmt->name->literal, functionFrame);
	}

	void Compiler::CompileLambdaExpr(LambdaExpr* stmt, Frame* frame)
	{
		Frame* lambdaFrame = new Frame(frame);

		lambdaFrame->AddOpCode(OP_ENTER_SCOPE);

		for (int64_t i = stmt->parameters.size() - 1; i >= 0; --i)
			CompileIdentifierExpr(stmt->parameters[i], lambdaFrame, INIT);

		for (const auto& s : stmt->body->stmts)
			CompileStmt(s, lambdaFrame);

		lambdaFrame->AddOpCode(OP_EXIT_SCOPE);

		frame->AddOpCode(OP_NEW_LAMBDA);
		size_t offset = frame->AddIntNum(frame->AddLambdaFrame(lambdaFrame));
		frame->AddOpCode(offset);
	}

	void Compiler::CompileFieldStmt(FieldStmt* stmt, Frame* frame)
	{
		Frame* fieldFrame = new Frame(frame);

		fieldFrame->AddOpCode(OP_ENTER_SCOPE);

		for (const auto& containedField : stmt->containedFields)
		{
			CompileIdentifierExpr(containedField, fieldFrame, READ);
			IdentifierExpr* instanceIdetExpr = new IdentifierExpr(containedFieldPrefixID + containedField->literal);
			CompileIdentifierExpr(instanceIdetExpr, fieldFrame, INIT);
		}


		for (auto& letStmt : stmt->letStmts)
		{
			for (auto& variable : letStmt->variables)
				if (variable.second->Type() == AST_LAMBDA)
					((LambdaExpr*)variable.second)->parameters.emplace_back(new IdentifierExpr("this"));//add 'this' parameter for field lambda function

			CompileLetStmt(letStmt, fieldFrame);
		}


		for (const auto& functionStmt : stmt->fnStmts)
		{
			functionStmt->parameters.emplace_back(new IdentifierExpr("this")); //regisiter field instance to function

			CompileFunctionStmt(functionStmt, fieldFrame);
		}

		fieldFrame->AddOpCode(OP_NEW_FIELD);
		uint64_t offset = fieldFrame->AddString(stmt->name);
		fieldFrame->AddOpCode(offset);

		fieldFrame->AddOpCode(OP_RETURN);

		frame->AddFieldFrame(stmt->name, fieldFrame);
	}

	void Compiler::CompileExpr(Expr* expr, Frame* frame, ObjectState state)
	{
		switch (expr->Type())
		{
		case AST_REAL:
			CompileRealNumExpr((RealNumExpr*)expr, frame);
			break;
		case AST_INT:
			CompileIntNumExpr((IntNumExpr*)expr, frame);
			break;
		case AST_STR:
			CompileStrExpr((StrExpr*)expr, frame);
			break;
		case AST_BOOL:
			CompileBoolExpr((BoolExpr*)expr, frame);
			break;
		case AST_NULL:
			CompileNullExpr((NullExpr*)expr, frame);
			break;
		case AST_IDENTIFIER:
			CompileIdentifierExpr((IdentifierExpr*)expr, frame, state);
			break;
		case AST_GROUP:
			CompileGroupExpr((GroupExpr*)expr, frame);
			break;
		case AST_ARRAY:
			CompileArrayExpr((ArrayExpr*)expr, frame);
			break;
		case AST_TABLE:
			CompileTableExpr((TableExpr*)expr, frame);
			break;
		case AST_INDEX:
			CompileIndexExpr((IndexExpr*)expr, frame, state);
			break;
		case AST_PREFIX:
			CompilePrefixExpr((PrefixExpr*)expr, frame);
			break;
		case AST_INFIX:
			CompileInfixExpr((InfixExpr*)expr, frame);
			break;
		case AST_CONDITION:
			CompileConditionExpr((ConditionExpr*)expr, frame);
			break;
		case AST_LAMBDA:
			CompileLambdaExpr((LambdaExpr*)expr, frame);
			break;
		case AST_FUNCTION_CALL:
			CompileFunctionCallExpr((FunctionCallExpr*)expr, frame);
			break;
		case AST_FIELD_CALL:
			CompileFieldCallExpr((FieldCallExpr*)expr, frame, state);
			break;
		case AST_REF:
			CompileRefExpr((RefExpr*)expr, frame);
			break;
		default:
			break;
		}
	}

	void Compiler::CompileIntNumExpr(IntNumExpr* expr, Frame* frame)
	{
		frame->AddOpCode(OP_NEW_INT);
		size_t offset = frame->AddIntNum(expr->value);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileRealNumExpr(RealNumExpr* expr, Frame* frame)
	{
		frame->AddOpCode(OP_NEW_REAL);
		size_t offset = frame->AddRealNum(expr->value);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileStrExpr(StrExpr* expr, Frame* frame)
	{
		frame->AddOpCode(OP_NEW_STR);
		size_t offset = frame->AddString(expr->value);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileBoolExpr(BoolExpr* expr, Frame* frame)
	{
		if (expr->value)
			frame->AddOpCode(OP_NEW_TRUE);
		else
			frame->AddOpCode(OP_NEW_FALSE);
	}

	void Compiler::CompileNullExpr(NullExpr* expr, Frame* frame)
	{
		frame->AddOpCode(OP_NEW_NULL);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr* expr, Frame* frame, ObjectState state)
	{
		if (state == READ)
			frame->AddOpCode(OP_GET_VAR);
		else if (state == WRITE)
			frame->AddOpCode(OP_SET_VAR);
		else if (state == INIT)
			frame->AddOpCode(OP_NEW_VAR);
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

	void Compiler::CompileGroupExpr(GroupExpr* expr, Frame* frame)
	{
		CompileExpr(expr->expr, frame);
	}

	void Compiler::CompileArrayExpr(ArrayExpr* expr, Frame* frame)
	{
		for (const auto& e : expr->elements)
			CompileExpr(e, frame);

		frame->AddOpCode(OP_NEW_ARRAY);
		size_t offset = frame->AddIntNum((int64_t)expr->elements.size());
		frame->AddOpCode(offset);
	}

	void Compiler::CompileTableExpr(TableExpr* expr, Frame* frame)
	{
		for (auto [key, value] : expr->elements)
		{
			CompileExpr(value, frame);
			CompileExpr(key, frame);
		}
		frame->AddOpCode(OP_NEW_TABLE);
		uint64_t offset = frame->AddIntNum((int64_t)expr->elements.size());
		frame->AddOpCode(offset);
	}

	void Compiler::CompileIndexExpr(IndexExpr* expr, Frame* frame, ObjectState state)
	{
		CompileExpr(expr->ds, frame);
		CompileExpr(expr->index, frame);
		if (state == READ)
			frame->AddOpCode(OP_GET_INDEX_VAR);
		else if (state == WRITE)
			frame->AddOpCode(OP_SET_INDEX_VAR);
	}

	void Compiler::CompilePrefixExpr(PrefixExpr* expr, Frame* frame)
	{
		CompileExpr(expr->right, frame);
		if (expr->op == "-")
			frame->AddOpCode(OP_NEG);
		else if (expr->op == "~")
			frame->AddOpCode(OP_BIT_NOT);
		else if (expr->op == "!")
			frame->AddOpCode(OP_NOT);
	}

	void Compiler::CompileInfixExpr(InfixExpr* expr, Frame* frame)
	{
		if (expr->op == "=")
		{
			CompileExpr(expr->right, frame);
			if (expr->right->Type() == AST_INFIX && ((InfixExpr*)expr->right)->op == "=") //continuous assignment such as a=b=c;
				CompileExpr(((InfixExpr*)expr->right)->left, frame);

			CompileExpr(expr->left, frame, WRITE);
		}
		else
		{
			CompileExpr(expr->right, frame);
			CompileExpr(expr->left, frame);

			if (expr->op == "+")
				frame->AddOpCode(OP_ADD);
			else if (expr->op == "-")
				frame->AddOpCode(OP_SUB);
			else if (expr->op == "*")
				frame->AddOpCode(OP_MUL);
			else if (expr->op == "/")
				frame->AddOpCode(OP_DIV);
			else if (expr->op == "%")
				frame->AddOpCode(OP_MOD);
			else if (expr->op == "&")
				frame->AddOpCode(OP_BIT_AND);
			else if (expr->op == "|")
				frame->AddOpCode(OP_BIT_OR);
			else if (expr->op == "^")
				frame->AddOpCode(OP_BIT_XOR);
			else if (expr->op == "&&")
				frame->AddOpCode(OP_AND);
			else if (expr->op == "||")
				frame->AddOpCode(OP_OR);
			else if (expr->op == ">")
				frame->AddOpCode(OP_GREATER);
			else if (expr->op == "<")
				frame->AddOpCode(OP_LESS);
			else if (expr->op == ">=")
			{
				frame->AddOpCode(OP_LESS);
				frame->AddOpCode(OP_NOT);
			}
			else if (expr->op == "<=")
			{
				frame->AddOpCode(OP_GREATER);
				frame->AddOpCode(OP_NOT);
			}
			else if (expr->op == "==")
				frame->AddOpCode(OP_EQUAL);
			else if (expr->op == "!=")
			{
				frame->AddOpCode(OP_EQUAL);
				frame->AddOpCode(OP_NOT);
			}
			else if (expr->op == "<<")
				frame->AddOpCode(OP_BIT_LEFT_SHIFT);
			else if (expr->op == ">>")
				frame->AddOpCode(OP_BIT_RIGHT_SHIFT);
			else if (expr->op == "+=")
			{
				frame->AddOpCode(OP_ADD);
				CompileExpr(expr->left, frame, WRITE);
			}
			else if (expr->op == "-=")
			{
				frame->AddOpCode(OP_SUB);
				CompileExpr(expr->left, frame, WRITE);
			}
			else if (expr->op == "*=")
			{
				frame->AddOpCode(OP_MUL);
				CompileExpr(expr->left, frame, WRITE);
			}
			else if (expr->op == "/=")
			{
				frame->AddOpCode(OP_DIV);
				CompileExpr(expr->left, frame, WRITE);
			}
			else if (expr->op == "&=")
			{
				frame->AddOpCode(OP_BIT_AND);
				CompileExpr(expr->left, frame, WRITE);
			}
			else if (expr->op == "|=")
			{
				frame->AddOpCode(OP_BIT_OR);
				CompileExpr(expr->left, frame, WRITE);
			}
			else if (expr->op == "^=")
			{
				frame->AddOpCode(OP_BIT_XOR);
				CompileExpr(expr->left, frame, WRITE);
			}
			else if (expr->op == "<<=")
			{
				frame->AddOpCode(OP_BIT_LEFT_SHIFT);
				CompileExpr(expr->left, frame, WRITE);
			}
			else if (expr->op == ">>=")
			{
				frame->AddOpCode(OP_BIT_RIGHT_SHIFT);
				CompileExpr(expr->left, frame, WRITE);
			}
			else
				Assert("Unknown binary op:" + expr->op);
		}
	}

	void Compiler::CompileRefExpr(RefExpr* expr, Frame* frame)
	{
		CompileExpr(expr->refExpr, frame);
		frame->AddOpCode(OP_REF);
	}

	void Compiler::CompileConditionExpr(ConditionExpr* expr, Frame* frame)
	{
		CompileExpr(expr->falseBranch, frame);
		CompileExpr(expr->trueBranch, frame);
		CompileExpr(expr->condition, frame);
		frame->AddOpCode(OP_CONDITION);
	}

	void Compiler::CompileFunctionCallExpr(FunctionCallExpr* expr, Frame* frame)
	{

		for (const auto& arg : expr->arguments)
			CompileExpr(arg, frame);

		//extra args such as 'this' or the field contained field instance 
		int64_t extraArgCount = 0;
		if (expr->name->Type() == AST_FIELD_CALL)
		{
			CompileRefExpr(new RefExpr(((FieldCallExpr*)expr->name)->callee), frame);
			extraArgCount++;
		}
		//argument count
		frame->AddOpCode(OP_NEW_INT);
		uint64_t offset = frame->AddIntNum((int64_t)expr->arguments.size() + extraArgCount);
		frame->AddOpCode(offset);

		if (expr->name->Type() == AST_IDENTIFIER && !LibraryManager::HasNativeFunction(((IdentifierExpr*)expr->name)->literal))
		{
			IdentifierExpr* tmpIden = new IdentifierExpr(((IdentifierExpr*)expr->name)->literal + std::to_string(expr->arguments.size()));
			CompileExpr(tmpIden, frame, FUNCTION_READ);
		}
		else if (expr->name->Type() == AST_FIELD_CALL)
		{
			FieldCallExpr* fieldCallExpr = (FieldCallExpr*)expr->name;

			CompileExpr(fieldCallExpr->callee, frame);

			if (fieldCallExpr->callMember->Type() == AST_FIELD_CALL) //continuous field call such as a.b.c;
				CompileExpr(((FieldCallExpr*)fieldCallExpr->callMember)->callee, frame, FIELD_MEMBER_READ);

			IdentifierExpr* tmpIden = new IdentifierExpr(((IdentifierExpr*)fieldCallExpr->callMember)->literal + std::to_string(expr->arguments.size()));
			CompileExpr(tmpIden, frame, FIELD_FUNCTION_READ);
		}
		else
			CompileExpr(expr->name, frame, FUNCTION_READ);

		frame->AddOpCode(OP_FUNCTION_CALL);
	}

	void Compiler::CompileFieldCallExpr(FieldCallExpr* expr, Frame* frame, ObjectState state)
	{
		CompileExpr(expr->callee, frame);

		if (expr->callMember->Type() == AST_FIELD_CALL) //continuous field call such as a.b.c;
			CompileExpr(((FieldCallExpr*)expr->callMember)->callee, frame, FIELD_MEMBER_READ);

		if (state == READ)
			CompileExpr(expr->callMember, frame, FIELD_MEMBER_READ);
		else if (state == WRITE)
			CompileExpr(expr->callMember, frame, FIELD_MEMBER_WRITE);
	}
}