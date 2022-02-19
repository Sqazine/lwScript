#pragma once
#include <vector>
#include <string>
#include "Ast.h"
#include "Frame.h"
#include "Object.h"
namespace lws
{
	enum ObjectState
	{
		INIT,
		READ,
		WRITE,
		FUNCTION_READ,
		FIELD_FUNCTION_READ,
		FIELD_MEMBER_READ,
		FIELD_MEMBER_WRITE,
	};
	class Compiler
	{
	public:
		Compiler();
		~Compiler();

		Frame *Compile(Stmt *stmt);

		void ResetStatus();
	private:
		void CompileAstStmts(AstStmts *stmt, Frame *frame);
		void CompileStmt(Stmt *stmt, Frame *frame);
		void CompileReturnStmt(ReturnStmt *stmt, Frame *frame);
		void CompileExprStmt(ExprStmt *stmt, Frame *frame);
		void CompileLetStmt(LetStmt *stmt, Frame *frame);
		void CompileScopeStmt(ScopeStmt *stmt, Frame *frame);
		void CompileIfStmt(IfStmt *stmt, Frame *frame);
		void CompileWhileStmt(WhileStmt *stmt, Frame *frame);
		void CompileFunctionStmt(FunctionStmt* stmt, Frame* frame);
		void CompileFieldStmt(FieldStmt *stmt, Frame *frame);

		void CompileExpr(Expr *expr, Frame *frame, ObjectState state = READ);
		void CompileIntNumExpr(IntNumExpr *expr, Frame *frame);
		void CompileRealNumExpr(RealNumExpr *expr, Frame *frame);
		void CompileStrExpr(StrExpr *expr, Frame *frame);
		void CompileBoolExpr(BoolExpr *expr, Frame *frame);
		void CompileNilExpr(NilExpr *expr, Frame *frame);
		void CompileIdentifierExpr(IdentifierExpr *expr, Frame *frame, ObjectState state = READ);
		void CompileGroupExpr(GroupExpr *expr, Frame *frame);
		void CompileArrayExpr(ArrayExpr *expr, Frame *frame);
		void CompileTableExpr(TableExpr *expr, Frame *frame);
		void CompileIndexExpr(IndexExpr *expr, Frame *frame, ObjectState state = READ);
		void CompilePrefixExpr(PrefixExpr *expr, Frame *frame);
		void CompileInfixExpr(InfixExpr *expr, Frame *frame);
		void CompileRefExpr(RefExpr *expr, Frame *frame);
		void CompileLambdaExpr(LambdaExpr *stmt, Frame *frame);
		void CompileConditionExpr(ConditionExpr *expr, Frame *frame);
		void CompileFunctionCallExpr(FunctionCallExpr *expr, Frame *frame);
		void CompileFieldCallExpr(FieldCallExpr *expr, Frame *frame, ObjectState state = READ);

		Frame* m_RootFrame;
	};
}