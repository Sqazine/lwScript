#pragma once
#include "Chunk.h"
#include "Ast.h"
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
		enum class SymbolLocation
		{
			GLOBAL,
			LOCAL,
			UPVALUE,
		};

		struct UpValue
		{
			uint8_t index = 0;
			uint8_t location = 0;
			uint8_t depth = -1;
		};

		struct FunctionSymbolInfo
		{
			int8_t paramCount = -1;
			VarArg varArg = VarArg::NONE;
		};

		struct Symbol
		{
			STD_STRING name;
			SymbolLocation location = SymbolLocation::GLOBAL;
			Privilege privilege = Privilege::MUTABLE;
			uint8_t index = 0;
			int8_t scopeDepth = -1;
			FunctionSymbolInfo functionSymInfo;
			UpValue upvalue; // available only while type is SymbolLocation::UPVALUE
			bool isCaptured = false;
			const Token *relatedToken;
		};
		class SymbolTable
		{
		public:
			SymbolTable();
			SymbolTable(SymbolTable *enclosing);
			~SymbolTable();

			Symbol Define(const Token *relatedToken, Privilege privilege, const STD_STRING &name, const FunctionSymbolInfo &functionInfo = {});

			Symbol Resolve(const Token *relatedToken, const STD_STRING &name, int8_t paramCount = -1, int8_t d = 0);

			std::array<Symbol, UINT8_COUNT> mSymbols;
			uint8_t mSymbolCount;
			uint8_t mGlobalSymbolCount;
			uint8_t mLocalSymbolCount;
			std::array<UpValue, UINT8_COUNT> mUpValues;
			int32_t mUpValueCount;
			uint8_t mScopeDepth; // Depth of scope nesting(related to code {} scope)
			SymbolTable *enclosing;

		private:
			UpValue AddUpValue(const Token *relatedToken, uint8_t location, uint8_t depth);
			uint8_t mTableDepth; // Depth of symbol table nesting(related to symboltable's enclosing)
		};

		void CompileDecl(Decl *decl);
		void CompileVarDecl(VarDecl * decl);
		void CompileFunctionDecl(FunctionDecl * decl);
		void CompileClassDecl(ClassDecl * decl);
		void CompileEnumDecl(EnumDecl * decl);
		void CompileModuleDecl(ModuleDecl * decl);

		void CompileDeclAndStmt(Stmt *stmt);

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
		void CompileLiteralExpr(LiteralExpr *expr);
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
		void CompileStructExpr(StructExpr *expr);
		void CompileVarArgExpr(VarArgExpr *expr, const RWState &state = RWState::READ);
		void CompileFactorialExpr(FactorialExpr *expr, const RWState &state = RWState::READ);

		Symbol CompileFunction(FunctionDecl * decl,ClassDecl::FunctionKind kind=ClassDecl::FunctionKind::NONE);
		uint32_t CompileVars(VarDecl * decl, bool IsInClassOrModuleScope);
		Symbol CompileClass(ClassDecl * decl);

		uint64_t EmitOpCode(OpCode opCode, const Token *token);
		uint64_t Emit(uint8_t opcode);
		uint64_t EmitConstant(const Value &value, const Token *token);
		uint64_t EmitClosure(FunctionObject *function, const Token *token);
		uint64_t EmitReturn(uint8_t retCount, const Token *token);
		uint64_t EmitJump(OpCode opcode, const Token *token);
		void EmitLoop(uint16_t opcode, const Token *token);
		void PatchJump(uint64_t offset);
		uint8_t AddConstant(const Value &value);

		void EmitSymbol(const Symbol &symbol);

		void EnterScope();
		void ExitScope();

		Chunk &CurChunk();
		FunctionObject *CurFunction();
		OpCodes &CurOpCodes();

		VarArg GetVarArgFromParameterList(const std::vector<VarDescExpr *> &parameterList);

		std::vector<Expr *> StatsPostfixExprs(AstNode *astNode);

		std::vector<FunctionObject *> mFunctionList;
		SymbolTable *mSymbolTable;

		int64_t mCurBreakStmtAddress, mCurContinueStmtAddress;
	};
}