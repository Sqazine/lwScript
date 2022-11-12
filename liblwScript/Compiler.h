#pragma once
#include "Chunk.h"
#include "Ast.h"
#include "SymbolTable.h"
#include "Object.h"
namespace lws
{
	enum class RWState // read write state
	{
		READ,
		WRITE,
	};

	class Compiler
	{
	public:
		Compiler();
		~Compiler();

		FunctionObject *Compile(Stmt *stmt);

		void ResetStatus();

	private:
		void CompileDeclaration(Stmt *stmt);
		void CompileDeclaration(Stmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress);
		void CompileLetDeclaration(LetStmt *stmt);
		void CompileConstDeclaration(ConstStmt *stmt);
		void CompileFunctionDeclaration(FunctionStmt *stmt);
		void CompileClassDeclaration(ClassStmt *stmt);

		void CompileStmt(Stmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress);
		void CompileExprStmt(ExprStmt *stmt);
		void CompileIfStmt(IfStmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress);
		void CompileScopeStmt(ScopeStmt *stmt, int64_t &breakStmtAddress, int64_t &continueStmtAddress);
		void CompileWhileStmt(WhileStmt *stmt);
		void CompileReturnStmt(ReturnStmt *stmt);
		void CompileBreakStmt(int64_t &stmtAddress);
		void CompileContinueStmt(int64_t &stmtAddress);

		void CompileExpr(Expr *expr, const RWState &state = RWState::READ);
		void CompileInfixExpr(InfixExpr *expr);
		void CompileIntNumExpr(IntNumExpr *expr);
		void CompileRealNumExpr(RealNumExpr *expr);
		void CompileBoolExpr(BoolExpr *expr);
		void CompilePrefixExpr(PrefixExpr *expr);
		void CompilePostfixExpr(PostfixExpr *expr,const RWState &state = RWState::READ, bool isDelayCompile = true);
		void CompileStrExpr(StrExpr *expr);
		void CompileNullExpr(NullExpr *expr);
		void CompileGroupExpr(GroupExpr *expr);
		void CompileArrayExpr(ArrayExpr *expr);
		void CompileTableExpr(TableExpr *expr);
		void CompileIndexExpr(IndexExpr *expr, const RWState &state = RWState::READ);
		void CompileNewExpr(NewExpr *expr);
		void CompileThisExpr(ThisExpr *expr);
		void CompileBaseExpr(BaseExpr *expr);
		void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state);
		void CompileLambdaExpr(LambdaExpr *expr);
		void CompileCallExpr(CallExpr *expr);
		void CompileDotExpr(DotExpr *expr, const RWState &state = RWState::READ);
		void CompileRefExpr(RefExpr *expr);

		Symbol CompileFunction(FunctionStmt *stmt);

		uint8_t Emit(uint8_t opcode);
		uint8_t EmitConstant(const Value &value);
		uint8_t EmitReturn(uint8_t retCount);
		uint8_t EmitJump(uint8_t opcode);
		void EmitLoop(uint16_t opcode);
		void PatchJump(uint8_t offset);
		uint8_t AddConstant(const Value &value);

		void EnterScope();
		void ExitScope();

		Chunk &CurChunk();
		FunctionObject *CurFunction();
		OpCodes &CurOpCodes();

		std::vector<Expr *> StatsPostfixExprs(AstNode *astNode);

		std::vector<FunctionObject *> mFunctionList;
		SymbolTable *mSymbolTable;
	};
}