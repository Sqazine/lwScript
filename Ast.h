#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Type.h"
#include "Token.h"
#include "Utils.h"
namespace lwscript
{
	enum class AstKind
	{
		// expr
		LITERAL,
		IDENTIFIER,
		VAR_DESC,
		GROUP,
		ARRAY,
		DICT,
		PREFIX,
		INFIX,
		POSTFIX,
		CONDITION,
		INDEX,
		REF,
		LAMBDA,
		DOT,
		CALL,
		NEW,
		THIS,
		BASE,
		COMPOUND,
		STRUCT,
		VAR_ARG,
		FACTORIAL,
		APPREGATE,
		// stmt
		VAR,
		EXPR,
		RETURN,
		IF,
		SCOPE,
		WHILE,
		BREAK,
		CONTINUE,
		ENUM,
		FUNCTION,
		CLASS,
		MODULE,
		ASTSTMTS,
	};

	struct AstNode
	{
		AstNode(Token *tagToken, AstKind kind) : tagToken(tagToken), kind(kind) {}
		virtual ~AstNode() {}
		virtual STD_STRING ToString() = 0;
		const AstKind kind;
		Token *tagToken;
	};

	struct Expr : public AstNode
	{
		Expr(Token *tagToken, AstKind kind) : AstNode(tagToken, kind) {}
		virtual ~Expr() {}
		virtual STD_STRING ToString() = 0;

		Type type;
	};

	struct LiteralExpr : public Expr
	{
		LiteralExpr(Token *tagToken);
		LiteralExpr(Token *tagToken, int64_t value);
		LiteralExpr(Token *tagToken, double value);
		LiteralExpr(Token *tagToken, bool value);
		LiteralExpr(Token *tagToken, STD_STRING_VIEW value);
		~LiteralExpr() override;
		STD_STRING ToString() override;

		union
		{
			int64_t i64Value;
			double f64Value;
			bool boolean;
			CHAR_T character;
			STD_STRING str;
		};
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr(Token *tagToken);
		IdentifierExpr(Token *tagToken, STD_STRING_VIEW literal);
		~IdentifierExpr() override;
		STD_STRING ToString() override;

		STD_STRING literal;
	};

	struct VarDescExpr : public Expr
	{
		VarDescExpr(Token *tagToken);
		VarDescExpr(Token *tagToken, const Type &type, Expr *name);
		~VarDescExpr() override;
		STD_STRING ToString() override;

		Expr *name;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr(Token *tagToken);
		ArrayExpr(Token *tagToken, const std::vector<Expr *> &elements);
		~ArrayExpr() override;
		STD_STRING ToString() override;

		std::vector<Expr *> elements;
	};

	struct DictExpr : public Expr
	{
		DictExpr(Token *tagToken);
		DictExpr(Token *tagToken, const std::vector<std::pair<Expr *, Expr *>> &elements);
		~DictExpr() override;
		STD_STRING ToString() override;

		std::vector<std::pair<Expr *, Expr *>> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr(Token *tagToken);
		GroupExpr(Token *tagToken, Expr *expr);
		~GroupExpr() override;
		STD_STRING ToString() override;

		Expr *expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr(Token *tagToken);
		PrefixExpr(Token *tagToken, STD_STRING_VIEW op, Expr *right);
		~PrefixExpr() override;
		STD_STRING ToString() override;

		STD_STRING op;
		Expr *right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr(Token *tagToken);
		InfixExpr(Token *tagToken, STD_STRING_VIEW op, Expr *left, Expr *right);
		~InfixExpr() override;
		STD_STRING ToString() override;

		STD_STRING op;
		Expr *left;
		Expr *right;
	};

	struct PostfixExpr : public Expr
	{
		PostfixExpr(Token *tagToken);
		PostfixExpr(Token *tagToken, Expr *left, STD_STRING_VIEW op);
		~PostfixExpr() override;
		STD_STRING ToString() override;

		Expr *left;
		STD_STRING op;
	};

	struct ConditionExpr : public Expr
	{
		ConditionExpr(Token *tagToken);
		ConditionExpr(Token *tagToken, Expr *condition, Expr *trueBranch, Expr *falseBranch);
		~ConditionExpr() override;
		STD_STRING ToString() override;

		Expr *condition;
		Expr *trueBranch;
		Expr *falseBranch;
	};

	struct IndexExpr : public Expr
	{
		IndexExpr(Token *tagToken);
		IndexExpr(Token *tagToken, Expr *ds, Expr *index);
		~IndexExpr() override;
		STD_STRING ToString() override;

		Expr *ds;
		Expr *index;
	};

	struct RefExpr : public Expr
	{
		RefExpr(Token *tagToken);
		RefExpr(Token *tagToken, Expr *refExpr);
		~RefExpr() override;
		STD_STRING ToString() override;

		Expr *refExpr;
	};

	struct LambdaExpr : public Expr
	{
		LambdaExpr(Token *tagToken);
		LambdaExpr(Token *tagToken, const std::vector<VarDescExpr *> &parameters, struct ScopeStmt *body);
		~LambdaExpr() override;
		STD_STRING ToString() override;

		std::vector<VarDescExpr *> parameters;
		struct ScopeStmt *body;
	};

	struct CallExpr : public Expr
	{
		CallExpr(Token *tagToken);
		CallExpr(Token *tagToken, Expr *callee, const std::vector<Expr *> &arguments);
		~CallExpr() override;
		STD_STRING ToString() override;

		Expr *callee;
		std::vector<Expr *> arguments;
	};

	struct DotExpr : public Expr
	{
		DotExpr(Token *tagToken);
		DotExpr(Token *tagToken, Expr *callee, IdentifierExpr *callMember);
		~DotExpr() override;
		STD_STRING ToString() override;

		Expr *callee;
		IdentifierExpr *callMember;
	};

	struct NewExpr : public Expr
	{
		NewExpr(Token *tagToken);
		NewExpr(Token *tagToken, Expr *callee);
		~NewExpr() override;
		STD_STRING ToString() override;

		Expr *callee;
	};

	struct ThisExpr : public Expr
	{
		ThisExpr(Token *tagToken);
		~ThisExpr() override;
		STD_STRING ToString() override;
	};

	struct BaseExpr : public Expr
	{
		BaseExpr(Token *tagToken, IdentifierExpr *callMember);
		~BaseExpr() override;
		STD_STRING ToString() override;

		IdentifierExpr *callMember;
	};

	struct CompoundExpr : public Expr
	{
		CompoundExpr(Token *tagToken);
		CompoundExpr(Token *tagToken, const std::vector<struct Stmt *> &stmts, Expr *endExpr);
		~CompoundExpr() override;
		STD_STRING ToString() override;

		std::vector<struct Stmt *> stmts;
		Expr *endExpr;
	};

	struct StructExpr : public Expr
	{
		StructExpr(Token *tagToken);
		StructExpr(Token *tagToken, const std::vector<std::pair<STD_STRING, Expr *>> &elements);
		~StructExpr() override;
		STD_STRING ToString() override;

		std::vector<std::pair<STD_STRING, Expr *>> elements;
	};

	struct VarArgExpr : public Expr
	{
		VarArgExpr(Token *tagToken);
		VarArgExpr(Token *tagToken, IdentifierExpr *argName);
		~VarArgExpr() override;
		STD_STRING ToString() override;
		IdentifierExpr *argName;
	};

	struct FactorialExpr : public Expr
	{
		FactorialExpr(Token *tagToken);
		FactorialExpr(Token *tagToken, Expr *expr);
		~FactorialExpr() override;
		STD_STRING ToString() override;
		Expr *expr;
	};

	struct AppregateExpr : public Expr
	{
		AppregateExpr(Token *tagToken);
		AppregateExpr(Token *tagToken, const std::vector<Expr *> &exprs);
		~AppregateExpr() override;
		STD_STRING ToString() override;
		std::vector<Expr *> exprs;
	};

	struct Stmt : public AstNode
	{
		Stmt(Token *tagToken, AstKind kind) : AstNode(tagToken, kind) {}
		virtual ~Stmt() {}
		virtual STD_STRING ToString() = 0;
	};

	struct ExprStmt : public Stmt
	{
		ExprStmt(Token *tagToken);
		ExprStmt(Token *tagToken, Expr *expr);
		~ExprStmt() override;
		STD_STRING ToString() override;

		Expr *expr;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt(Token *tagToken);
		ReturnStmt(Token *tagToken, Expr *expr);
		~ReturnStmt() override;
		STD_STRING ToString() override;

		Expr *expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt(Token *tagToken);
		IfStmt(Token *tagToken, Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
		~IfStmt() override;
		STD_STRING ToString() override;

		Expr *condition;
		Stmt *thenBranch;
		Stmt *elseBranch;
	};

	struct ScopeStmt : public Stmt
	{
		ScopeStmt(Token *tagToken);
		ScopeStmt(Token *tagToken, const std::vector<Stmt *> &stmts);
		~ScopeStmt() override;
		STD_STRING ToString() override;

		std::vector<Stmt *> stmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt(Token *tagToken);
		WhileStmt(Token *tagToken, Expr *condition, ScopeStmt *body, ScopeStmt *increment = nullptr);
		~WhileStmt() override;
		STD_STRING ToString() override;

		Expr *condition;
		ScopeStmt *body;
		ScopeStmt *increment;
	};

	struct BreakStmt : public Stmt
	{
		BreakStmt(Token *tagToken);
		~BreakStmt() override;
		STD_STRING ToString() override;
	};

	struct ContinueStmt : public Stmt
	{
		ContinueStmt(Token *tagToken);
		~ContinueStmt() override;
		STD_STRING ToString() override;
	};

		struct AstStmts : public Stmt
	{
		AstStmts(Token *tagToken);
		AstStmts(Token *tagToken, std::vector<Stmt *> stmts);
		~AstStmts() override;

		STD_STRING ToString() override;

		std::vector<Stmt *> stmts;
	};

	struct Decl : public Stmt
	{
		Decl(Token *tagToken, AstKind kind) : Stmt(tagToken, kind) {}
		virtual ~Decl() {}
		virtual STD_STRING ToString() = 0;
	};

	struct VarDecl : public Decl
	{
		VarDecl(Token *tagToken);
		VarDecl(Token *tagToken, Privilege privilege, const std::vector<std::pair<Expr *, Expr *>> &variables);
		~VarDecl() override;
		STD_STRING ToString() override;

		Privilege privilege;
		std::vector<std::pair<Expr *, Expr *>> variables;
	};

	struct EnumDecl : public Decl
	{
		EnumDecl(Token *tagToken);
		EnumDecl(Token *tagToken, IdentifierExpr *name, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems);
		~EnumDecl() override;

		STD_STRING ToString() override;

		IdentifierExpr *name;
		std::unordered_map<IdentifierExpr *, Expr *> enumItems;
	};

	enum class FunctionKind
	{
		CLASS_CONSTRUCTOR,
		CLASS_CLOSURE,
		FUNCTION,
	};

	struct FunctionDecl : public Decl
	{
		FunctionDecl(Token *tagToken);
		FunctionDecl(Token *tagToken, FunctionKind kind, IdentifierExpr *name, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body);
		~FunctionDecl() override;

		STD_STRING ToString() override;

		FunctionKind functionKind;
		IdentifierExpr *name;
		std::vector<VarDescExpr *> parameters;
		ScopeStmt *body;
		std::vector<Type> returnTypes;
	};

	struct ClassDecl : public Decl
	{
		ClassDecl(Token *tagToken);
		ClassDecl(Token *tagToken,
				  STD_STRING name,
				  const std::vector<VarDecl *> &varItems,
				  const std::vector<FunctionDecl *> &fnItems,
				  const std::vector<EnumDecl *> &enumItems,
				  const std::vector<FunctionDecl *> &constructors = {},
				  const std::vector<IdentifierExpr *> &parentClasses = {});
		~ClassDecl() override;

		STD_STRING ToString() override;

		STD_STRING name;
		std::vector<IdentifierExpr *> parentClasses;
		std::vector<FunctionDecl *> constructors;
		std::vector<VarDecl *> varItems;
		std::vector<FunctionDecl *> fnItems;
		std::vector<EnumDecl *> enumItems;
	};

	struct ModuleDecl : public Decl
	{
		
		ModuleDecl(Token *tagToken);
		ModuleDecl(Token *tagToken,
				   IdentifierExpr *name,
				   const std::vector<VarDecl *> &varItems,
				   const std::vector<ClassDecl *> &classItems,
				   const std::vector<ModuleDecl *> &moduleItems,
				   const std::vector<EnumDecl *> &enumItems,
				   const std::vector<FunctionDecl *> &functionItems);
		~ModuleDecl() override;

		STD_STRING ToString() override;

		IdentifierExpr *name;
		std::vector<VarDecl *> varItems;
		std::vector<ClassDecl *> classItems;
		std::vector<ModuleDecl *> moduleItems;
		std::vector<EnumDecl *> enumItems;
		std::vector<FunctionDecl *> functionItems;
	};
}