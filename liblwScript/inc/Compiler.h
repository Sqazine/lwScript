#pragma once
#include "Chunk.h"
#include "Ast.h"
#include "SymbolTable.h"
#include "Object.h"
namespace lwscript
{
	enum class RWState // read write state
	{
		READ,
		WRITE,
	};

	class LWSCRIPT_API Compiler
	{
	public:
		Compiler();
		~Compiler();

		FunctionObject *Compile(Stmt *stmt);

		void ResetStatus();

	private:
		void CompileDecl(Stmt *stmt);
		void CompileVarDecl(VarStmt *stmt);
		void CompileFunctionDecl(FunctionStmt *stmt);
		void CompileClassDecl(ClassStmt *stmt);
		void CompileEnumDecl(EnumStmt *stmt);
		void CompileModuleDecl(ModuleStmt *stmt);

		void CompileStmt(Stmt *stmt);
		void CompileExprStmt(ExprStmt *stmt);
		void CompileIfStmt(IfStmt *stmt);
		void CompileScopeStmt(ScopeStmt *stmt);
		void CompileWhileStmt(WhileStmt *stmt);
		void CompileReturnStmt(ReturnStmt *stmt);
		void CompileBreakStmt(BreakStmt *stmt);
		void CompileContinueStmt(ContinueStmt *stmt);

		void CompileExpr(Expr *expr, const RWState &state = RWState::READ, int8_t paramCount = -1);
		void CompileInfixExpr(InfixExpr *expr);
		void CompileLiteralExpr(LiteralExpr* expr);
		void CompilePrefixExpr(PrefixExpr *expr);
		void CompilePostfixExpr(PostfixExpr *expr, const RWState &state = RWState::READ, bool isDelayCompile = true);
		void CompileConditionExpr(ConditionExpr *expr);
		void CompileGroupExpr(GroupExpr *expr);
		void CompileArrayExpr(ArrayExpr *expr);
		void CompileAppregateExpr(AppregateExpr *expr);
		void CompileDictExpr(DictExpr *expr);
		void CompileIndexExpr(IndexExpr *expr, const RWState &state = RWState::READ);
		void CompileNewExpr(NewExpr *expr);
		void CompileThisExpr(ThisExpr *expr);
		void CompileBaseExpr(BaseExpr *expr);
		void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state, int8_t paramCount = -1);
		void CompileLambdaExpr(LambdaExpr *expr);
		void CompileCompoundExpr(CompoundExpr *expr);
		void CompileCallExpr(CallExpr *expr);
		void CompileDotExpr(DotExpr *expr, const RWState &state = RWState::READ);
		void CompileRefExpr(RefExpr *expr);
		void CompileAnonymousObjExpr(AnonyObjExpr *expr);
		void CompileVarArgExpr(VarArgExpr *expr, const RWState &state = RWState::READ);
		void CompileFactorialExpr(FactorialExpr *expr, const RWState &state = RWState::READ);

		Symbol CompileFunction(FunctionStmt *stmt);
		uint32_t CompileVars(VarStmt *stmt, bool IsInClassOrModuleScope);
		Symbol CompileClass(ClassStmt* stmt);

		uint64_t EmitOpCode(OpCode opCode,const Token* token);
		uint64_t Emit(uint8_t opcode);
		uint64_t EmitConstant(const Value &value, const Token* token);
		uint64_t EmitClosure(FunctionObject *function, const Token* token);
		uint64_t EmitReturn(uint8_t retCount, const Token* token);
		uint64_t EmitJump(OpCode opcode, const Token* token);
		void EmitLoop(uint16_t opcode, const Token* token);
		void PatchJump(uint64_t offset);
		uint8_t AddConstant(const Value &value);

		void EmitSymbol(const Symbol& symbol);

		void EnterScope();
		void ExitScope();

		Chunk &CurChunk();
		FunctionObject *CurFunction();
		OpCodes &CurOpCodes();

		std::vector<Expr *> StatsPostfixExprs(AstNode *astNode);

		std::vector<FunctionObject *> mFunctionList;
		SymbolTable *mSymbolTable;

		int64_t mCurBreakStmtAddress, mCurContinueStmtAddress;
	};
}