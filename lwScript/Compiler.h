#pragma once
#include <vector>
#include <string>
#include "Ast.h"
#include "Frame.h"
#include "Object.h"
namespace lwScript
{
	enum State
	{
		NONE,
		INIT,
		READ,
		WRITE,
		REF
	};
	class Compiler
	{
	public:
		Compiler();
		~Compiler();

		const Frame &Compile(Stmt *stmt);

		void ResetStatus();

	private:
		void CompileAstStmts(AstStmts *stmt, Frame &frame);
		void CompileStmt(Stmt *stmt, Frame &frame);
		void CompileReturnStmt(ReturnStmt *stmt, Frame &frame);
		void CompileExprStmt(ExprStmt *stmt, Frame &frame);
		void CompileLetStmt(LetStmt *stmt, Frame &frame);
		void CompileScopeStmt(ScopeStmt *stmt, Frame &frame);
		void CompileIfStmt(IfStmt *stmt, Frame &frame);
		void CompileWhileStmt(WhileStmt *stmt, Frame &frame);

		void CompileExpr(Expr *expr, Frame &frame, State state = READ);
		void CompileNumExpr(NumExpr *expr, Frame &frame);
		void CompileStrExpr(StrExpr *expr, Frame &frame);
		void CompileBoolExpr(BoolExpr *expr, Frame &frame);
		void CompileNilExpr(NilExpr *expr, Frame &frame);
		void CompileIdentifierExpr(IdentifierExpr *expr, Frame &frame, State state = READ);
		void CompileGroupExpr(GroupExpr *expr, Frame &frame);
		void CompileArrayExpr(ArrayExpr *expr, Frame &frame);
		void CompileIndexExpr(IndexExpr *expr, Frame &frame, State state = READ);
		void CompileFunctionExpr(FunctionExpr *expr, Frame &frame);
		void CompileStructExpr(StructExpr *expr, Frame &frame);
		void CompileRefExpr(RefExpr *expr, Frame &frame);
		void CompilePrefixExpr(PrefixExpr *expr, Frame &frame);
		void CompileInfixExpr(InfixExpr *expr, Frame &frame);
		void CompileFunctionCallExpr(FunctionCallExpr *expr, Frame &frame);

		Frame m_RootFrame;
	};

	Compiler::Compiler()
	{
	}
	Compiler::~Compiler()
	{
	}

	const Frame &Compiler::Compile(Stmt *stmt)
	{
		CompileAstStmts((AstStmts *)stmt, m_RootFrame);
		return m_RootFrame;
	}

	void Compiler::ResetStatus()
	{
		m_RootFrame.Clear();
	}

	void Compiler::CompileAstStmts(AstStmts *stmt, Frame &frame)
	{
		for (const auto &s : stmt->stmts)
			CompileStmt(s, frame);
	}

	void Compiler::CompileStmt(Stmt *stmt, Frame &frame)
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
		default:
			break;
		}
	}
	void Compiler::CompileReturnStmt(ReturnStmt *stmt, Frame &frame)
	{
		if (stmt->expr)
			CompileExpr(stmt->expr, frame);

		frame.AddOpCode(OP_RETURN);
	}

	void Compiler::CompileExprStmt(ExprStmt *stmt, Frame &frame)
	{
		CompileExpr(stmt->expr, frame);
	}

	void Compiler::CompileLetStmt(LetStmt *stmt, Frame &frame)
	{
		for (const auto &[key, value] : stmt->variables)
		{
			CompileExpr(value, frame);
			CompileExpr(key, frame, INIT);
		}
	}

	void Compiler::CompileScopeStmt(ScopeStmt *stmt, Frame &frame)
	{
		frame.AddOpCode(OP_ENTER_SCOPE);

		for (const auto &s : stmt->stmts)
			CompileStmt(s, frame);

		frame.AddOpCode(OP_EXIT_SCOPE);
	}

	void Compiler::CompileIfStmt(IfStmt *stmt, Frame &frame)
	{
		CompileExpr(stmt->condition, frame);

		auto jmpIfFalseAddress = new NumObject();
		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(jmpIfFalseAddress);
		frame.AddOpCode(offset);
		frame.AddOpCode(OP_JUMP_IF_FALSE);

		CompileStmt(stmt->thenBranch, frame);

		auto jmpAddress = new NumObject();
		frame.AddOpCode(OP_PUSH);
		offset = frame.AddObject(jmpAddress);
		frame.AddOpCode(offset);
		frame.AddOpCode(OP_JUMP);

		jmpIfFalseAddress->value = (double)frame.GetOpCodeSize() - 1.0;

		if (stmt->elseBranch)
			CompileStmt(stmt->elseBranch, frame);

		jmpAddress->value = (double)frame.GetOpCodeSize() - 1.0;
	}
	void Compiler::CompileWhileStmt(WhileStmt *stmt, Frame &frame)
	{
		auto jmpAddress = new NumObject();
		jmpAddress->value = (double)frame.GetOpCodeSize() - 1.0;
		CompileExpr(stmt->condition, frame);

		auto jmpIfFalseAddress = new NumObject();
		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(jmpIfFalseAddress);
		frame.AddOpCode(offset);
		frame.AddOpCode(OP_JUMP_IF_FALSE);

		CompileStmt(stmt->body, frame);

		frame.AddOpCode(OP_PUSH);
		offset = frame.AddObject(jmpAddress);
		frame.AddOpCode(offset);
		frame.AddOpCode(OP_JUMP);

		jmpIfFalseAddress->value = (double)frame.GetOpCodeSize() - 1.0;
	}

	void Compiler::CompileExpr(Expr *expr, Frame &frame, State state)
	{
		switch (expr->Type())
		{
		case AstType::NUM:
			CompileNumExpr((NumExpr *)expr, frame);
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
		case AstType::INDEX:
			CompileIndexExpr((IndexExpr *)expr, frame, state);
			break;
		case AstType::STRUCT:
			CompileStructExpr((StructExpr *)expr, frame);
			break;
		case AstType::FUNCTION:
			CompileFunctionExpr((FunctionExpr *)expr, frame);
			break;
		case AstType::REF:
			CompileRefExpr((RefExpr *)expr, frame);
			break;
		case AstType::PREFIX:
			CompilePrefixExpr((PrefixExpr *)expr, frame);
			break;
		case AstType::INFIX:
			CompileInfixExpr((InfixExpr *)expr, frame);
			break;
		case AstType::FUNCTION_CALL:
			CompileFunctionCallExpr((FunctionCallExpr*)expr,frame);
			break;
		default:
			break;
		}
	}

	void Compiler::CompileNumExpr(NumExpr *expr, Frame &frame)
	{
		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(new NumObject(expr->value));
		frame.AddOpCode(offset);
	}

	void Compiler::CompileStrExpr(StrExpr *expr, Frame &frame)
	{
		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(new StrObject(expr->value));
		frame.AddOpCode(offset);
	}

	void Compiler::CompileBoolExpr(BoolExpr *expr, Frame &frame)
	{
		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(expr->value ? trueObject : falseObject);
		frame.AddOpCode(offset);
	}

	void Compiler::CompileNilExpr(NilExpr *expr, Frame &frame)
	{
		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(nilObject);
		frame.AddOpCode(offset);
	}

	void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, Frame &frame, State state)
	{
		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(new StrObject(expr->literal));
		frame.AddOpCode(offset);
		if (state == READ)
			frame.AddOpCode(OP_GET_VAR);
		else if (state == WRITE)
			frame.AddOpCode(OP_SET_VAR);
		else if (state == INIT)
			frame.AddOpCode(OP_DEFINE_VAR);
		else if (state == REF)
			frame.AddOpCode(OP_REF_VAR);
	}

	void Compiler::CompileGroupExpr(GroupExpr *expr, Frame &frame)
	{
		CompileExpr(expr->expr, frame);
	}

	void Compiler::CompileArrayExpr(ArrayExpr *expr, Frame &frame)
	{
		for (const auto &e : expr->elements)
			CompileExpr(e, frame);

		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(new NumObject((double)expr->elements.size()));
		frame.AddOpCode(offset);
		frame.AddOpCode(OP_ARRAY);
	}

	void Compiler::CompileIndexExpr(IndexExpr *expr, Frame &frame, State state)
	{
		CompileExpr(expr->index, frame);
		CompileExpr(expr->array, frame);
		if (state == READ)
			frame.AddOpCode(OP_GET_INDEX_VAR);
		else if (state == WRITE)
			frame.AddOpCode(OP_SET_INDEX_VAR);
	}

	void Compiler::CompileFunctionExpr(FunctionExpr *expr, Frame &frame)
	{
		Frame functionFrame;

		functionFrame.AddOpCode(OP_ENTER_SCOPE);

		for (int64_t i=expr->parameters.size()-1;i>=0;--i)
			CompileIdentifierExpr(expr->parameters[i], functionFrame, INIT);

		CompileScopeStmt(expr->body, functionFrame);

		functionFrame.AddOpCode(OP_EXIT_SCOPE);

		frame.AddFunctionFrame(functionFrame);

		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(new FunctionObject((uint64_t)frame.GetFunctionFrameSize() - 1));
		frame.AddOpCode(offset);
	}

	void Compiler::CompileStructExpr(StructExpr *expr, Frame &frame)
	{
		uint8_t memberCount = 0;
		for (const auto &letStmt : expr->letStmts)
		{
			for (const auto &[key, value] : letStmt->variables)
			{
				CompileExpr(value, frame);
				frame.AddOpCode(OP_PUSH);
				uint8_t offset = frame.AddObject(new StrObject(key->literal));
				frame.AddOpCode(offset);
			}
			memberCount += (uint8_t)letStmt->variables.size();
		}

		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(new NumObject((double)memberCount));
		frame.AddOpCode(offset);

		frame.AddOpCode(OP_STRUCT);
	}

	void Compiler::CompileRefExpr(RefExpr *expr, Frame &frame)
	{
		CompileExpr(expr->expr, frame, REF);
	}

	void Compiler::CompilePrefixExpr(PrefixExpr *expr, Frame &frame)
	{
		CompileExpr(expr->right, frame);
		if (expr->op == "-")
			frame.AddOpCode(OP_NEG);
	}

	void Compiler::CompileInfixExpr(InfixExpr *expr, Frame &frame)
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
				frame.AddOpCode(OP_LOGIC_AND);
			else if (expr->op == "||")
				frame.AddOpCode(OP_LOGIC_OR);
			else if (expr->op == ">")
				frame.AddOpCode(OP_GREATER);
			else if (expr->op == "<")
				frame.AddOpCode(OP_LESS);
			else if (expr->op == ">=")
				frame.AddOpCode(OP_GREATER_EQUAL);
			else if (expr->op == "<=")
				frame.AddOpCode(OP_LESS_EQUAL);
			else if (expr->op == "==")
				frame.AddOpCode(OP_EQUAL);
			else if (expr->op == "!=")
				frame.AddOpCode(OP_NEQUAL);
			else
			{
				std::cout << "Unknown binary op:" << expr->op << std::endl;
				exit(1);
			}
		}
	}

	void Compiler::CompileFunctionCallExpr(FunctionCallExpr *expr, Frame &frame)
	{
		for(const auto& arg:expr->arguments)
			 CompileExpr(arg,frame);
		CompileExpr(expr->name,frame,NONE);

		//argumentc count
		frame.AddOpCode(OP_PUSH);
		uint8_t offset = frame.AddObject(new NumObject(expr->arguments.size()));
		frame.AddOpCode(offset);

		frame.AddOpCode(OP_FUNCTION_CALL);
	}
}