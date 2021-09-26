#include "Compiler.h"
#include "Utils.h"
namespace lws
{
	Compiler::Compiler()
	{
	}
	Compiler::~Compiler()
	{
	}

	const Frame& Compiler::Compile(Stmt* stmt)
	{
		CompileAstStmts((AstStmts*)stmt, m_RootFrame);
		return m_RootFrame;
	}

	void Compiler::ResetStatus()
	{
		m_RootFrame.Clear();
	}

	void Compiler::CompileAstStmts(AstStmts* stmt, Frame& frame)
	{
		for (const auto& s : stmt->stmts)
			CompileStmt(s, frame);
	}

	void Compiler::CompileStmt(Stmt* stmt, Frame& frame)
	{
		switch (stmt->Type())
		{
		case AstType::RETURN:
			CompileReturnStmt((ReturnStmt*)stmt, frame);
			break;
		case AstType::EXPR:
			CompileExprStmt((ExprStmt*)stmt, frame);
			break;
		case AstType::LET:
			CompileLetStmt((LetStmt*)stmt, frame);
			break;
		case AstType::SCOPE:
			CompileScopeStmt((ScopeStmt*)stmt, frame);
			break;
		case AstType::IF:
			CompileIfStmt((IfStmt*)stmt, frame);
			break;
		case AstType::WHILE:
			CompileWhileStmt((WhileStmt*)stmt, frame);
			break;
		default:
			break;
		}
	}
	void Compiler::CompileReturnStmt(ReturnStmt* stmt, Frame& frame)
	{
		if (stmt->expr)
			CompileExpr(stmt->expr, frame);

		frame.AddOpCode(OP_RETURN);
	}

	void Compiler::CompileExprStmt(ExprStmt* stmt, Frame& frame)
	{
		CompileExpr(stmt->expr, frame);
	}

	void Compiler::CompileLetStmt(LetStmt* stmt, Frame& frame)
	{
		for (const auto& [key, value] : stmt->variables)
		{
			CompileExpr(value, frame);
			CompileExpr(key, frame, INIT);
		}
	}

	void Compiler::CompileScopeStmt(ScopeStmt* stmt, Frame& frame)
	{
		frame.AddOpCode(OP_ENTER_SCOPE);

		for (const auto& s : stmt->stmts)
			CompileStmt(s, frame);

		frame.AddOpCode(OP_EXIT_SCOPE);
	}

	void Compiler::CompileIfStmt(IfStmt* stmt, Frame& frame)
	{
		CompileExpr(stmt->condition, frame);

		frame.AddOpCode(OP_JUMP_IF_FALSE);
		uint8_t jmpIfFalseOffset = frame.AddNumber(0);
		frame.AddOpCode(jmpIfFalseOffset);

		CompileStmt(stmt->thenBranch, frame);

		frame.AddOpCode(OP_JUMP);
		uint8_t jmpOffset = frame.AddNumber(0);
		frame.AddOpCode(jmpOffset);

		frame.GetNumbers()[jmpIfFalseOffset] = (double)frame.GetOpCodeSize() - 1.0;

		if (stmt->elseBranch)
			CompileStmt(stmt->elseBranch, frame);

		frame.GetNumbers()[jmpOffset] = (double)frame.GetOpCodeSize() - 1.0;
	}
	void Compiler::CompileWhileStmt(WhileStmt* stmt, Frame& frame)
	{
		uint8_t jmpAddress= (double)frame.GetOpCodeSize() - 1.0;
		CompileExpr(stmt->condition, frame);

		frame.AddOpCode(OP_JUMP_IF_FALSE);
		uint8_t jmpIfFalseOffset = frame.AddNumber(0);
		frame.AddOpCode(jmpIfFalseOffset);

		CompileStmt(stmt->body, frame);

		frame.AddOpCode(OP_JUMP);
		uint8_t offset = frame.AddNumber(jmpAddress);
		frame.AddOpCode(offset);

		frame.GetNumbers()[jmpIfFalseOffset] = (double)frame.GetOpCodeSize() - 1.0;
	}

	void Compiler::CompileExpr(Expr* expr, Frame& frame, ObjectState state)
	{
		switch (expr->Type())
		{
		case AstType::NUM:
			CompileNumExpr((NumExpr*)expr, frame);
			break;
		case AstType::STR:
			CompileStrExpr((StrExpr*)expr, frame);
			break;
		case AstType::BOOL:
			CompileBoolExpr((BoolExpr*)expr, frame);
			break;
		case AstType::NIL:
			CompileNilExpr((NilExpr*)expr, frame);
			break;
		case AstType::IDENTIFIER:
			CompileIdentifierExpr((IdentifierExpr*)expr, frame, state);
			break;
		case AstType::GROUP:
			CompileGroupExpr((GroupExpr*)expr, frame);
			break;
		case AstType::ARRAY:
			CompileArrayExpr((ArrayExpr*)expr, frame);
			break;
		case AstType::INDEX:
			CompileIndexExpr((IndexExpr*)expr, frame, state);
			break;
		case AstType::FUNCTION:
			CompileFunctionExpr((FunctionExpr*)expr, frame);
			break;
		case AstType::PREFIX:
			CompilePrefixExpr((PrefixExpr*)expr, frame);
			break;
		case AstType::INFIX:
			CompileInfixExpr((InfixExpr*)expr, frame);
			break;
		case AstType::FUNCTION_CALL:
			CompileFunctionCallExpr((FunctionCallExpr*)expr, frame);
			break;
		default:
			break;
		}
	}

	void Compiler::CompileNumExpr(NumExpr* expr, Frame& frame)
	{
		frame.AddOpCode(OP_NUM);
		uint8_t offset = frame.AddNumber(expr->value);
		frame.AddOpCode(offset);
	}

	void Compiler::CompileStrExpr(StrExpr* expr, Frame& frame)
	{
		frame.AddOpCode(OP_STR);
		uint8_t offset = frame.AddString(expr->value);
		frame.AddOpCode(offset);
	}

	void Compiler::CompileBoolExpr(BoolExpr* expr, Frame& frame)
	{
		frame.AddOpCode(OP_BOOL);
		uint8_t offset = frame.AddBoolean(expr->value);
		frame.AddOpCode(offset);
	}

	void Compiler::CompileNilExpr(NilExpr* expr, Frame& frame)
	{
		frame.AddOpCode(OP_NIL);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr* expr, Frame& frame, ObjectState state)
	{
		if (state == READ)
			frame.AddOpCode(OP_GET_VAR);
		else if (state == WRITE)
			frame.AddOpCode(OP_SET_VAR);
		else if (state == INIT)
			frame.AddOpCode(OP_DEFINE_VAR);
		uint8_t offset = frame.AddString(expr->literal);
		frame.AddOpCode(offset);
	}

	void Compiler::CompileGroupExpr(GroupExpr* expr, Frame& frame)
	{
		CompileExpr(expr->expr, frame);
	}

	void Compiler::CompileArrayExpr(ArrayExpr* expr, Frame& frame)
	{
		for (const auto& e : expr->elements)
			CompileExpr(e, frame);

		frame.AddOpCode(OP_NUM);
		uint8_t offset = frame.AddNumber((double)expr->elements.size());
		frame.AddOpCode(offset);
		frame.AddOpCode(OP_DEFINE_ARRAY);
	}

	void Compiler::CompileIndexExpr(IndexExpr* expr, Frame& frame, ObjectState state)
	{
		CompileExpr(expr->index, frame);
		CompileExpr(expr->array, frame);
		if (state == READ)
			frame.AddOpCode(OP_GET_INDEX_VAR);
		else if (state == WRITE)
			frame.AddOpCode(OP_SET_INDEX_VAR);
	}

	void Compiler::CompileFunctionExpr(FunctionExpr* expr, Frame& frame)
	{
		Frame functionFrame;

		functionFrame.AddOpCode(OP_ENTER_SCOPE);

		for (int64_t i = expr->parameters.size() - 1; i >= 0; --i)
			CompileIdentifierExpr(expr->parameters[i], functionFrame, INIT);

		CompileScopeStmt(expr->body, functionFrame);

		functionFrame.AddOpCode(OP_EXIT_SCOPE);

		frame.AddFunctionFrame(functionFrame);

		frame.AddOpCode(OP_FUNCTION);
		uint8_t offset = frame.AddNumber((double)frame.GetFunctionFrameSize() - 1.0);
		frame.AddOpCode(offset);
	}

	void Compiler::CompilePrefixExpr(PrefixExpr* expr, Frame& frame)
	{
		CompileExpr(expr->right, frame);
		if (expr->op == "-")
			frame.AddOpCode(OP_NEG);
	}

	void Compiler::CompileInfixExpr(InfixExpr* expr, Frame& frame)
	{
		if (expr->op == "=")
		{
			CompileExpr(expr->right, frame);
			CompileExpr(expr->left, frame, WRITE);
		}
		else
		{
			CompileExpr(expr->right, frame);
			CompileExpr(expr->left, frame);

			if (expr->op == "+")
				frame.AddOpCode(OP_ADD);
			else if (expr->op == "-")
				frame.AddOpCode(OP_SUB);
			else if (expr->op == "*")
				frame.AddOpCode(OP_MUL);
			else if (expr->op == "/")
				frame.AddOpCode(OP_DIV);
			else if (expr->op == "&&")
				frame.AddOpCode(OP_AND);
			else if (expr->op == "||")
				frame.AddOpCode(OP_OR);
			else if (expr->op == ">")
				frame.AddOpCode(OP_GT);
			else if (expr->op == "<")
				frame.AddOpCode(OP_LE);
			else if (expr->op == ">=")
				frame.AddOpCode(OP_GTEQ);
			else if (expr->op == "<=")
				frame.AddOpCode(OP_LEEQ);
			else if (expr->op == "==")
				frame.AddOpCode(OP_EQ);
			else if (expr->op == "!=")
				frame.AddOpCode(OP_NEQ);
			else
				Assert("Unknown binary op:" + expr->op);
		}
	}

	void Compiler::CompileFunctionCallExpr(FunctionCallExpr* expr, Frame& frame)
	{
		for (const auto& arg : expr->arguments)
			CompileExpr(arg, frame);
		CompileExpr(expr->name, frame);

		//argument count
		frame.AddOpCode(OP_NUM);
		uint8_t offset = frame.AddNumber((double)expr->arguments.size());
		frame.AddOpCode(offset);

		frame.AddOpCode(OP_FUNCTION_CALL);
	}
}