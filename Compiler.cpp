#include "Compiler.h"
#include "Object.h"
#include "Constant.h"
#include <iostream>
namespace lwScript
{
	Compiler::Compiler()
	{
	}
	Compiler::~Compiler()
	{
	}

	const Chunk &Compiler::Compile(Stmt *stmt)
	{
		CompileAstStmts((AstStmts *)stmt);
		return m_Chunk;
	}

	void Compiler::ResetStatus()
	{
		m_Chunk.Clear();
	}

	void Compiler::CompileAstStmts(AstStmts *stmt)
	{
		for (const auto &s : stmt->stmts)
			CompileStmt(s);
	}

	void Compiler::CompileStmt(Stmt *stmt)
	{
		switch (stmt->Type())
		{
		case AstType::RETURN:
			CompileReturnStmt((ReturnStmt *)stmt);
			break;
		case AstType::EXPR:
			CompileExprStmt((ExprStmt *)stmt);
			break;
		case AstType::LET:
			CompileLetStmt((LetStmt *)stmt);
			break;
		case AstType::SCOPE:
			CompileScopeStmt((ScopeStmt*) stmt);
			break;
		default:
			break;
		}
	}
	void Compiler::CompileReturnStmt(ReturnStmt *stmt)
	{
		if (stmt->expr)
			CompileExpr(stmt->expr);

		m_Chunk.AddOpCode(OP_RETURN);
	}

	void Compiler::CompileExprStmt(ExprStmt *stmt)
	{
		CompileExpr(stmt->expr);
	}

	void Compiler::CompileLetStmt(LetStmt *stmt)
	{
		for (const auto &[key, value] : stmt->variables)
		{
			CompileExpr(value);
			CompileExpr(key, INIT);
		}
	}

	void Compiler::CompileScopeStmt(ScopeStmt *stmt)
	{
		m_Chunk.AddOpCode(OP_ENTER_SCOPE);

		for(const auto& s:stmt->stmts)
			CompileStmt(s);
		
		m_Chunk.AddOpCode(OP_EXIT_SCOPE);
	}

	void Compiler::CompileExpr(Expr *expr, State state)
	{
		switch (expr->Type())
		{
	
		case AstType::NUM:
			CompileNumExpr((NumExpr *)expr);
			break;
		case AstType::STR:
			CompileStrExpr((StrExpr *)expr);
			break;
		case AstType::BOOL:
			CompileBoolExpr((BoolExpr *)expr);
			break;
		case AstType::NIL:
			CompileNilExpr((NilExpr *)expr);
			break;
		case AstType::IDENTIFIER:
			CompileIdentifierExpr((IdentifierExpr *)expr, state);
			break;
		case AstType::GROUP:
			CompileGroupExpr((GroupExpr *)expr);
			break;
		case AstType::ARRAY:
			CompileArrayExpr((ArrayExpr *)expr);
			break;
		case AstType::INDEX:
			CompileIndexExpr((IndexExpr *)expr, state);
			break;
		case AstType::PREFIX:
			CompilePrefixExpr((PrefixExpr *)expr);
			break;
		case AstType::INFIX:
			CompileInfixExpr((InfixExpr *)expr);
			break;
		default:
			break;
		}
	}

	void Compiler::CompileNumExpr(NumExpr *expr)
	{
		m_Chunk.AddOpCode(OP_PUSH);
		uint8_t offset = m_Chunk.AddObject(new NumObject(expr->value));
		m_Chunk.AddOpCode(offset);
	}

	void Compiler::CompileStrExpr(StrExpr *expr)
	{
		m_Chunk.AddOpCode(OP_PUSH);
		uint8_t offset = m_Chunk.AddObject(new StrObject(expr->value));
		m_Chunk.AddOpCode(offset);
	}

	void Compiler::CompileBoolExpr(BoolExpr *expr)
	{
		m_Chunk.AddOpCode(OP_PUSH);
		uint8_t offset = m_Chunk.AddObject(expr->value ? trueObject : falseObject);
		m_Chunk.AddOpCode(offset);
	}

	void Compiler::CompileNilExpr(NilExpr *expr)
	{
		m_Chunk.AddOpCode(OP_PUSH);
		uint8_t offset = m_Chunk.AddObject(nilObject);
		m_Chunk.AddOpCode(offset);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, State state)
	{
		m_Chunk.AddOpCode(OP_PUSH);
		uint8_t offset = m_Chunk.AddObject(new StrObject(expr->literal));
		m_Chunk.AddOpCode(offset);
		if (state == READ)
			m_Chunk.AddOpCode(OP_GET_VAR);
		else if (state == WRITE)
			m_Chunk.AddOpCode(OP_SET_VAR);
		else if (state == INIT)
			m_Chunk.AddOpCode(OP_DEFINE_VAR);
	}

	void Compiler::CompileGroupExpr(GroupExpr *expr)
	{
		CompileExpr(expr->expr);
	}

	void Compiler::CompileArrayExpr(ArrayExpr *expr)
	{
		for (const auto &e : expr->elements)
			CompileExpr(e);

		m_Chunk.AddOpCode(OP_PUSH);
		uint8_t offset = m_Chunk.AddObject(new NumObject((double)expr->elements.size()));
		m_Chunk.AddOpCode(offset);
		m_Chunk.AddOpCode(OP_ARRAY);
	}

	void Compiler::CompileIndexExpr(IndexExpr *expr, State state)
	{
		CompileExpr(expr->index);
		CompileExpr(expr->array);
		if (state == READ)
			m_Chunk.AddOpCode(OP_GET_INDEX_VAR);
		else if (state == WRITE)
			m_Chunk.AddOpCode(OP_SET_INDEX_VAR);
	}

	void Compiler::CompilePrefixExpr(PrefixExpr *expr)
	{
		CompileExpr(expr->right);
		if (expr->op == "-")
			m_Chunk.AddOpCode(OP_NEG);
	}

	void Compiler::CompileInfixExpr(InfixExpr *expr)
	{
		if (expr->op == "=")
		{
			CompileExpr(expr->right);
			CompileExpr(expr->left, WRITE);
		}
		else
		{
			CompileExpr(expr->right);
			CompileExpr(expr->left);

			if (expr->op == "+")
				m_Chunk.AddOpCode(OP_ADD);
			else if (expr->op == "-")
				m_Chunk.AddOpCode(OP_SUB);
			else if (expr->op == "*")
				m_Chunk.AddOpCode(OP_MUL);
			else if (expr->op == "/")
				m_Chunk.AddOpCode(OP_DIV);
			else if (expr->op == "&&")
				m_Chunk.AddOpCode(OP_LOGIC_AND);
			else if (expr->op == "||")
				m_Chunk.AddOpCode(OP_LOGIC_OR);
			else if (expr->op == ">")
				m_Chunk.AddOpCode(OP_GREATER);
			else if (expr->op == "<")
				m_Chunk.AddOpCode(OP_LESS);
			else if (expr->op == ">=")
				m_Chunk.AddOpCode(OP_GREATER_EQUAL);
			else if (expr->op == "<=")
				m_Chunk.AddOpCode(OP_LESS_EQUAL);
			else if (expr->op == "==")
				m_Chunk.AddOpCode(OP_EQUAL);
			else if (expr->op == "!=")
				m_Chunk.AddOpCode(OP_NEQUAL);
			else
			{
			std::cout<<"Unknown binary op:" << expr->op<<std::endl;
				exit(1);
			}
		}
	}
}