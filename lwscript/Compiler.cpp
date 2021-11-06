#include "Compiler.h"
#include "Utils.h"
namespace lws
{
	Compiler::Compiler()
		:m_RootFrame(new Frame())
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

	Frame *Compiler::Compile(Stmt *stmt)
	{
		CompileAstStmts((AstStmts *)stmt, m_RootFrame);
		return m_RootFrame;
	}

	void Compiler::ResetStatus()
	{
		m_RootFrame->Clear();
	}

	void Compiler::CompileAstStmts(AstStmts *stmt, Frame *frame)
	{
		for (const auto &s : stmt->stmts)
			CompileStmt(s, frame);
	}

	void Compiler::CompileStmt(Stmt *stmt, Frame *frame)
	{
		switch (stmt->Type())
		{
		case AstType::RETURN:
			CompileReturnStmt((ReturnStmt *)stmt, frame);
			break;
		case AstType::EXPR:
			CompileExprStmt((ExprStmt *)stmt, frame);
			break;
		case AstType::LET:
			CompileLetStmt((LetStmt *)stmt, frame);
			break;
		case AstType::SCOPE:
			CompileScopeStmt((ScopeStmt *)stmt, frame);
			break;
		case AstType::IF:
			CompileIfStmt((IfStmt *)stmt, frame);
			break;
		case AstType::WHILE:
			CompileWhileStmt((WhileStmt *)stmt, frame);
			break;
		case AstType::CLASS:
			CompileClassStmt((ClassStmt *)stmt, frame);
			break;
		default:
			break;
		}
	}
	void Compiler::CompileReturnStmt(ReturnStmt *stmt, Frame *frame)
	{
		if (stmt->expr)
			CompileExpr(stmt->expr, frame);

		frame->AddOpCode(OP_RETURN);
	}

	void Compiler::CompileExprStmt(ExprStmt *stmt, Frame *frame)
	{
		CompileExpr(stmt->expr, frame);
	}

	void Compiler::CompileLetStmt(LetStmt *stmt, Frame *frame)
	{
		for (auto [key, value] : stmt->variables)
		{
			CompileExpr(value, frame);
			CompileExpr(key, frame, INIT);
		}
	}

	void Compiler::CompileScopeStmt(ScopeStmt *stmt, Frame *frame)
	{
		frame->AddOpCode(OP_ENTER_SCOPE);

		for (const auto &s : stmt->stmts)
			CompileStmt(s, frame);

		frame->AddOpCode(OP_EXIT_SCOPE);
	}

	void Compiler::CompileIfStmt(IfStmt *stmt, Frame *frame)
	{
		CompileExpr(stmt->condition, frame);

		frame->AddOpCode(OP_JUMP_IF_FALSE);
		uint64_t jmpIfFalseOffset = frame->AddIntegerNum(0);
		frame->AddOpCode(jmpIfFalseOffset);

		CompileStmt(stmt->thenBranch, frame);

		frame->AddOpCode(OP_JUMP);
		uint64_t jmpOffset = frame->AddIntegerNum(0);
		frame->AddOpCode(jmpOffset);

		frame->GetIntegerNums()[jmpIfFalseOffset] = (double)frame->GetOpCodeSize() - 1.0;

		if (stmt->elseBranch)
			CompileStmt(stmt->elseBranch, frame);

		frame->GetIntegerNums()[jmpOffset] = (double)frame->GetOpCodeSize() - 1.0;
	}
	void Compiler::CompileWhileStmt(WhileStmt *stmt, Frame *frame)
	{
		uint64_t jmpAddress = frame->GetOpCodeSize() - 1;
		CompileExpr(stmt->condition, frame);

		frame->AddOpCode(OP_JUMP_IF_FALSE);
		uint64_t jmpIfFalseOffset = frame->AddIntegerNum(0);
		frame->AddOpCode(jmpIfFalseOffset);

		CompileStmt(stmt->body, frame);

		frame->AddOpCode(OP_JUMP);
		uint64_t offset = frame->AddIntegerNum(jmpAddress);
		frame->AddOpCode(offset);

		frame->GetIntegerNums()[jmpIfFalseOffset] = (double)frame->GetOpCodeSize() - 1.0;
	}

	void Compiler::CompileFunctionExpr(FunctionExpr *stmt, Frame *frame)
	{
		Frame *functionFrame = new Frame(frame);

		functionFrame->AddOpCode(OP_ENTER_SCOPE);

		for (int64_t i = stmt->parameters.size() - 1; i >= 0; --i)
			CompileIdentifierExpr(stmt->parameters[i], functionFrame, INIT);

		CompileScopeStmt(stmt->body, functionFrame);

		functionFrame->AddOpCode(OP_EXIT_SCOPE);

		frame->AddOpCode(OP_NEW_FUNCTION);
		size_t offset = frame->AddIntegerNum(frame->AddFunctionFrame(functionFrame));
		frame->AddOpCode(offset);
	}

	void Compiler::CompileClassStmt(ClassStmt *stmt, Frame *frame)
	{
		Frame *classFrame = new Frame(frame);

		classFrame->AddOpCode(OP_ENTER_SCOPE);

		for (const auto letStmt : stmt->letStmts)
			CompileLetStmt(letStmt, classFrame);

		frame->AddClassFrame(stmt->name, classFrame);
	}

	void Compiler::CompileExpr(Expr *expr, Frame *frame, ObjectState state)
	{
		switch (expr->Type())
		{
		case AstType::FLOATING:
			CompileFloatingExpr((FloatingExpr *)expr, frame);
			break;
		case AstType::INTEGER:
			CompileIntegerExpr((IntegerExpr *)expr, frame);
			break;
		case AstType::STR:
			CompileStrExpr((StrExpr *)expr, frame);
			break;
		case AstType::BOOL:
			CompileBoolExpr((BoolExpr *)expr, frame);
			break;
		case AstType::NIL:
			CompileNilExpr((NilExpr *)expr, frame);
			break;
		case AstType::IDENTIFIER:
			CompileIdentifierExpr((IdentifierExpr *)expr, frame, state);
			break;
		case AstType::GROUP:
			CompileGroupExpr((GroupExpr *)expr, frame);
			break;
		case AstType::ARRAY:
			CompileArrayExpr((ArrayExpr *)expr, frame);
			break;
		case AstType::TABLE:
			CompileTableExpr((TableExpr *)expr, frame);
			break;
		case AstType::INDEX:
			CompileIndexExpr((IndexExpr *)expr, frame, state);
			break;
		case AstType::PREFIX:
			CompilePrefixExpr((PrefixExpr *)expr, frame);
			break;
		case AstType::INFIX:
			CompileInfixExpr((InfixExpr *)expr, frame);
			break;
		case AstType::CONDITION:
			CompileConditionExpr((ConditionExpr *)expr, frame);
			break;
		case AstType::FUNCTION:
			CompileFunctionExpr((FunctionExpr*)expr, frame);
			break;
		case AstType::FUNCTION_CALL:
			CompileFunctionCallExpr((FunctionCallExpr *)expr, frame);
			break;
		case AstType::CLASS_CALL:
			CompileClassCallExpr((ClassCallExpr *)expr, frame, state);
			break;
		case AstType::REF:
			CompileRefExpr((RefExpr *)expr, frame);
			break;
		case AstType::NEW:
			CompileNewExpr((NewExpr*)expr, frame);
			break;
		default:
			break;
		}
	}

	void Compiler::CompileIntegerExpr(IntegerExpr *expr, Frame *frame)
	{
		frame->AddOpCode(OP_NEW_INTEGER);
		size_t offset = frame->AddIntegerNum(expr->value);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileFloatingExpr(FloatingExpr *expr, Frame *frame)
	{
		frame->AddOpCode(OP_NEW_FLOATING);
		size_t offset = frame->AddFloatingNum(expr->value);
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

	void Compiler::CompileNilExpr(NilExpr *expr, Frame *frame)
	{
		frame->AddOpCode(OP_NEW_NIL);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, Frame *frame, ObjectState state)
	{
		if (state == READ)
			frame->AddOpCode(OP_GET_VAR);
		else if (state == WRITE)
			frame->AddOpCode(OP_SET_VAR);
		else if (state == INIT)
			frame->AddOpCode(OP_DEFINE_VAR);
		else if (state == CLASS_READ)
			frame->AddOpCode(OP_CLASS_CALL);
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
		size_t offset = frame->AddIntegerNum((int64_t)expr->elements.size());
		frame->AddOpCode(offset);
	}

	void Compiler::CompileTableExpr(TableExpr *expr, Frame *frame)
	{
		for (auto [key, value] : expr->elements)
		{
			CompileExpr(value, frame);
			CompileExpr(key, frame);
		}
		frame->AddOpCode(OP_NEW_TABLE);
		uint64_t offset = frame->AddIntegerNum((int64_t)expr->elements.size());
		frame->AddOpCode(offset);
	}

	void Compiler::CompileIndexExpr(IndexExpr *expr, Frame *frame, ObjectState state)
	{
		CompileExpr(expr->ds, frame);
		CompileExpr(expr->index, frame);
		if (state == READ)
			frame->AddOpCode(OP_GET_INDEX_VAR);
		else if (state == WRITE)
			frame->AddOpCode(OP_SET_INDEX_VAR);
	}

	void Compiler::CompilePrefixExpr(PrefixExpr *expr, Frame *frame)
	{
		CompileExpr(expr->right, frame);
		if (expr->op == "-")
			frame->AddOpCode(OP_NEG);
		else if (expr->op == "~")
			frame->AddOpCode(OP_BIT_NOT);
		else if (expr->op == "!")
			frame->AddOpCode(OP_NOT);
	}

	void Compiler::CompileInfixExpr(InfixExpr *expr, Frame *frame)
	{
		if (expr->op == "=")
		{
			CompileExpr(expr->right, frame);
			if (expr->right->Type() == AstType::INFIX && ((InfixExpr*)expr->right)->op == "=")//continuous assignment such as a=b=c;
				CompileExpr(((InfixExpr*)expr->right)->left, frame);
			
			if(expr->left->Type()== AstType::CLASS_CALL)
				CompileExpr(expr->left, frame, CLASS_WRITE);
			else
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
				frame->AddOpCode(OP_GREATER_EQUAL);
			else if (expr->op == "<=")
				frame->AddOpCode(OP_LESS_EQUAL);
			else if (expr->op == "==")
				frame->AddOpCode(OP_EQUAL);
			else if (expr->op == "!=")
				frame->AddOpCode(OP_NOT_EQUAL);
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

	void Compiler::CompileRefExpr(RefExpr *expr, Frame *frame)
	{
		CompileExpr(expr->refExpr,frame);
		frame->AddOpCode(OP_REF);
	}

	void Compiler::CompileNewExpr(NewExpr* expr, Frame* frame)
	{
		frame->AddOpCode(OP_NEW_CLASS);
		uint64_t offset = frame->AddString(expr->callee->literal);
		frame->AddOpCode(offset);
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

		//argument count
		frame->AddOpCode(OP_NEW_INTEGER);
		uint64_t offset = frame->AddIntegerNum((int64_t)expr->arguments.size());
		frame->AddOpCode(offset);

		frame->AddOpCode(OP_FUNCTION_CALL);
		offset = frame->AddString(expr->name);
		frame->AddOpCode(offset);
	}

	void Compiler::CompileClassCallExpr(ClassCallExpr *expr, Frame *frame, ObjectState state)
	{
		CompileExpr(expr->callee, frame, CLASS_READ);
		CompileExpr(expr->callMember, frame, state);
	}
}