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
		CONST_INIT,
		VAR_INIT,
		VAR_READ,
		VAR_WRITE,
		FUNCTION_READ,
		CLASS_FUNCTION_READ,
		CLASS_MEMBER_READ,
		CLASS_MEMBER_WRITE,
	};
	class Compiler
	{
	public:
		Compiler();
		~Compiler();

		Frame *Compile(Stmt *stmt);

		void ResetStatus();

	private:
		enum class ReferenceType
		{
			VARIABLE,
			OBJECT
		};

		void CompileAstStmts(AstStmts *stmt, Frame *frame);
		void CompileStmt(Stmt *stmt, Frame *frame, uint64_t breakStmtAddressOffset = 0, uint64_t continueStmtAddressOffset = 0);
		void CompileReturnStmt(ReturnStmt *stmt, Frame *frame);
		void CompileExprStmt(ExprStmt *stmt, Frame *frame);
		void CompileLetStmt(LetStmt *stmt, Frame *frame);
		void CompileConstStmt(ConstStmt *stmt, Frame *frame);
		void CompileScopeStmt(ScopeStmt *stmt, Frame *frame, uint64_t breakStmtAddressOffset = 0, uint64_t continueStmtAddressOffset = 0);
		void CompileIfStmt(IfStmt *stmt, Frame *frame, uint64_t breakStmtAddressOffset = 0, uint64_t continueStmtAddressOffset = 0);
		void CompileWhileStmt(WhileStmt *stmt, Frame *frame);
		void CompileBreakStmt(uint64_t addressOffset, Frame *frame);
		void CompileContinueStmt(uint64_t addressOffset, Frame *frame);
		void CompileEnumStmt(EnumStmt *enumStmt, Frame *frame);
		void CompileFunctionStmt(FunctionStmt *stmt, Frame *frame);
		void CompileClassStmt(ClassStmt *stmt, Frame *frame);

		void CompileExpr(Expr *expr, Frame *frame, ObjectState state = VAR_READ);
		void CompileIntNumExpr(IntNumExpr *expr, Frame *frame);
		void CompileRealNumExpr(RealNumExpr *expr, Frame *frame);
		void CompileStrExpr(StrExpr *expr, Frame *frame);
		void CompileBoolExpr(BoolExpr *expr, Frame *frame);
		void CompileNullExpr(NullExpr *expr, Frame *frame);
		void CompileIdentifierExpr(IdentifierExpr *expr, Frame *frame, ObjectState state = VAR_READ);
		void CompileGroupExpr(GroupExpr *expr, Frame *frame);
		void CompileArrayExpr(ArrayExpr *expr, Frame *frame);
		void CompileTableExpr(TableExpr *expr, Frame *frame);
		void CompileIndexExpr(IndexExpr *expr, Frame *frame, ObjectState state = VAR_READ);
		void CompilePrefixExpr(PrefixExpr *expr, Frame *frame);
		void CompileInfixExpr(InfixExpr *expr, Frame *frame);
		void CompilePostfixExpr(PostfixExpr *expr, Frame *frame, bool isDelayCompile = true,ObjectState state = VAR_READ);
		void CompileRefExpr(RefExpr *expr, Frame *frame, ReferenceType type = ReferenceType::VARIABLE);
		void CompileLambdaExpr(LambdaExpr *stmt, Frame *frame);
		void CompileConditionExpr(ConditionExpr *expr, Frame *frame);
		void CompileFunctionCallExpr(FunctionCallExpr *expr, Frame *frame);
		void CompileClassCallExpr(ClassCallExpr *expr, Frame *frame, ObjectState state = VAR_READ);

		void CompileBreakAndContinueStmt(uint64_t addressOffset, Frame *frame);

		std::vector<Expr *> StatsPostfixExprs(AstNode *astNode);

		Frame *mRootFrame;
	};
}