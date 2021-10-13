#include "Compiler.h"
#include "Utils.h"

Compiler::Compiler()
{
}
Compiler::~Compiler()
{
}

Frame* Compiler::Compile(Stmt* stmt)
{
	Frame* frame = new Frame();
	CompileAstStmts((AstStmts*)stmt, frame);
	return frame;
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
	case AstType::FUNCTION:
		CompileFunctionStmt((FunctionStmt*)stmt, frame);
		break;
	case AstType::STRUCT:
		CompileStructStmt((StructStmt*)stmt, frame);
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
	uint64_t jmpIfFalseOffset = frame->AddIntegerNum(0);
	frame->AddOpCode(jmpIfFalseOffset);

	CompileStmt(stmt->thenBranch, frame);

	frame->AddOpCode(OP_JUMP);
	uint64_t jmpOffset = frame->AddIntegerNum(0);
	frame->AddOpCode(jmpOffset);

	frame->GetFloatingNums()[jmpIfFalseOffset] = (double)frame->GetOpCodeSize() - 1.0;

	if (stmt->elseBranch)
		CompileStmt(stmt->elseBranch, frame);

	frame->GetFloatingNums()[jmpOffset] = (double)frame->GetOpCodeSize() - 1.0;
}
void Compiler::CompileWhileStmt(WhileStmt* stmt, Frame* frame)
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

	frame->GetFloatingNums()[jmpIfFalseOffset] = (double)frame->GetOpCodeSize() - 1.0;
}

void Compiler::CompileFunctionStmt(FunctionStmt* stmt, Frame* frame)
{
	Frame* functionFrame = new Frame(frame);

	functionFrame->AddOpCode(OP_ENTER_SCOPE);

	for (int64_t i = stmt->parameters.size() - 1; i >= 0; --i)
		CompileIdentifierExpr(stmt->parameters[i], functionFrame, INIT);

	CompileScopeStmt(stmt->body, functionFrame);

	functionFrame->AddOpCode(OP_EXIT_SCOPE);

	frame->AddFunctionFrame(stmt->name, functionFrame);
}

void Compiler::CompileStructStmt(StructStmt* stmt, Frame* frame)
{
	Frame* structFrame = new Frame(frame);

	structFrame->AddOpCode(OP_ENTER_SCOPE);

	for (const auto letStmt : stmt->letStmts)
		CompileLetStmt(letStmt, structFrame);

	structFrame->AddOpCode(OP_DEFINE_STRUCT);

	structFrame->AddOpCode(OP_RETURN);

	frame->AddStructFrame(stmt->name, structFrame);

}

void Compiler::CompileExpr(Expr* expr, Frame* frame, ObjectState state)
{
	switch (expr->Type())
	{
	case AstType::FLOATING:
		CompileFloatingExpr((FloatingExpr*)expr, frame);
		break;
	case AstType::INTEGER:
		CompileIntegerExpr((IntegerExpr*)expr, frame);
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
	case AstType::TABLE:
		CompileTableExpr((TableExpr*)expr, frame);
		break;
	case AstType::INDEX:
		CompileIndexExpr((IndexExpr*)expr, frame, state);
		break;
	case AstType::PREFIX:
		CompilePrefixExpr((PrefixExpr*)expr, frame);
		break;
	case AstType::INFIX:
		CompileInfixExpr((InfixExpr*)expr, frame);
		break;
	case AstType::CONDITION:
		CompileConditionExpr((ConditionExpr*)expr,frame);
		break;
	case AstType::FUNCTION_CALL:
		CompileFunctionCallExpr((FunctionCallExpr*)expr, frame);
		break;
	case AstType::STRUCT_CALL:
		CompileStructCallExpr((StructCallExpr*)expr, frame, state);
		break;
	case AstType::REF:
		CompileRefExpr((RefExpr*)expr,frame);
		break;
	default:
		break;
	}
}

void Compiler::CompileIntegerExpr(IntegerExpr* expr, Frame* frame)
{
	frame->AddOpCode(OP_INTEGER);
	size_t offset = frame->AddIntegerNum(expr->value);
	frame->AddOpCode(offset);
}

void Compiler::CompileFloatingExpr(FloatingExpr* expr, Frame* frame)
{
	frame->AddOpCode(OP_FLOATING);
	size_t offset = frame->AddFloatingNum(expr->value);
	frame->AddOpCode(offset);
}

void Compiler::CompileStrExpr(StrExpr* expr, Frame* frame)
{
	frame->AddOpCode(OP_STR);
	size_t offset = frame->AddString(expr->value);
	frame->AddOpCode(offset);
}

void Compiler::CompileBoolExpr(BoolExpr* expr, Frame* frame)
{
	if (expr->value)
		frame->AddOpCode(OP_TRUE);
	else
		frame->AddOpCode(OP_FALSE);
}

void Compiler::CompileNilExpr(NilExpr* expr, Frame* frame)
{
	frame->AddOpCode(OP_NIL);
}

void Compiler::CompileIdentifierExpr(IdentifierExpr* expr, Frame* frame, ObjectState state)
{
	if (state == READ)
		frame->AddOpCode(OP_GET_VAR);
	else if (state == WRITE)
		frame->AddOpCode(OP_SET_VAR);
	else if (state == INIT)
		frame->AddOpCode(OP_DEFINE_VAR);
	else if (state == STRUCT_READ)
		frame->AddOpCode(OP_GET_STRUCT);
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

	frame->AddOpCode(OP_DEFINE_ARRAY);
	size_t offset = frame->AddIntegerNum((int64_t)expr->elements.size());
	frame->AddOpCode(offset);
}

void Compiler::CompileTableExpr(TableExpr* expr, Frame* frame)
{
	for (auto [key, value] : expr->elements)
	{
		CompileExpr(value, frame);
		CompileExpr(key, frame);
	}
	frame->AddOpCode(OP_DEFINE_TABLE);
	uint64_t offset = frame->AddIntegerNum((int64_t)expr->elements.size());
	frame->AddOpCode(offset);
}

void Compiler::CompileIndexExpr(IndexExpr* expr, Frame* frame, ObjectState state)
{
	CompileExpr(expr->index, frame);
	CompileExpr(expr->array, frame);
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

void Compiler::CompileRefExpr(RefExpr* expr, Frame* frame)
{
	frame->AddOpCode(OP_REF);
	uint64_t offset = frame->AddString(expr->refName);
	frame->AddOpCode(offset);
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

	//argument count
	frame->AddOpCode(OP_INTEGER);
	uint64_t offset = frame->AddIntegerNum((int64_t)expr->arguments.size());
	frame->AddOpCode(offset);

	frame->AddOpCode(OP_FUNCTION_CALL);
	offset = frame->AddString(expr->name);
	frame->AddOpCode(offset);
}

void Compiler::CompileStructCallExpr(StructCallExpr* expr, Frame* frame, ObjectState state)
{
	CompileExpr(expr->callee, frame, STRUCT_READ);
	CompileExpr(expr->callMember, frame, state);

	frame->AddOpCode(OP_END_GET_STRUCT);
}
