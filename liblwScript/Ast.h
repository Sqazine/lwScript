#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
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
		ANONY_OBJ,
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

#define AST_TEMPLATE(name, base)          \
	struct name : public base             \
	{                                     \
		name(Token *tagToken);            \
		~name() override;                 \
		std::wstring ToString() override; \
	};

	struct AstNode
	{
		AstNode(Token *tagToken, AstKind kind) : tagToken(tagToken), kind(kind) {}
		virtual ~AstNode() {}
		virtual std::wstring ToString() = 0;
		const AstKind kind;
		Token *tagToken;
	};

	struct Expr : public AstNode
	{
		Expr(Token *tagToken, AstKind kind) : AstNode(tagToken, kind) {}
		virtual ~Expr() {}
		virtual std::wstring ToString() = 0;
	};

	struct LiteralExpr : public Expr
	{
		enum class Type
		{
			NIL,
			INTEGER,
			FLOATING,
			BOOLEAN,
			CHARACTER,
			STRING
		};

		LiteralExpr(Token *tagToken);
		LiteralExpr(Token *tagToken, int64_t value);
		LiteralExpr(Token *tagToken, double value);
		LiteralExpr(Token *tagToken, bool value);
		LiteralExpr(Token *tagToken, std::wstring_view value);
		~LiteralExpr() override;
		std::wstring ToString() override;

		Type literalType;

		union
		{
			int64_t iValue;
			double dValue;
			bool boolean;
			wchar_t character;
			std::wstring str;
		};
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr(Token *tagToken);
		IdentifierExpr(Token *tagToken, std::wstring_view literal);
		~IdentifierExpr() override;
		std::wstring ToString() override;

		std::wstring literal;
	};

	struct VarDescExpr : public Expr
	{
		VarDescExpr(Token *tagToken);
		VarDescExpr(Token *tagToken, std::wstring_view typeDesc, Expr *name);
		~VarDescExpr() override;
		std::wstring ToString() override;

		std::wstring typeDesc;
		Expr *name;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr(Token *tagToken);
		ArrayExpr(Token *tagToken, const std::vector<Expr *> &elements);
		~ArrayExpr() override;
		std::wstring ToString() override;

		std::vector<Expr *> elements;
	};

	struct DictExpr : public Expr
	{
		DictExpr(Token *tagToken);
		DictExpr(Token *tagToken, const std::vector<std::pair<Expr *, Expr *>> &elements);
		~DictExpr() override;
		std::wstring ToString() override;

		std::vector<std::pair<Expr *, Expr *>> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr(Token *tagToken);
		GroupExpr(Token *tagToken, Expr *expr);
		~GroupExpr() override;
		std::wstring ToString() override;

		Expr *expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr(Token *tagToken);
		PrefixExpr(Token *tagToken, std::wstring_view op, Expr *right);
		~PrefixExpr() override;
		std::wstring ToString() override;

		std::wstring op;
		Expr *right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr(Token *tagToken);
		InfixExpr(Token *tagToken, std::wstring_view op, Expr *left, Expr *right);
		~InfixExpr() override;
		std::wstring ToString() override;

		std::wstring op;
		Expr *left;
		Expr *right;
	};

	struct PostfixExpr : public Expr
	{
		PostfixExpr(Token *tagToken);
		PostfixExpr(Token *tagToken, Expr *left, std::wstring_view op);
		~PostfixExpr() override;
		std::wstring ToString() override;

		Expr *left;
		std::wstring op;
	};

	struct ConditionExpr : public Expr
	{
		ConditionExpr(Token *tagToken);
		ConditionExpr(Token *tagToken, Expr *condition, Expr *trueBranch, Expr *falseBranch);
		~ConditionExpr() override;
		std::wstring ToString() override;

		Expr *condition;
		Expr *trueBranch;
		Expr *falseBranch;
	};

	struct IndexExpr : public Expr
	{
		IndexExpr(Token *tagToken);
		IndexExpr(Token *tagToken, Expr *ds, Expr *index);
		~IndexExpr() override;
		std::wstring ToString() override;

		Expr *ds;
		Expr *index;
	};

	struct RefExpr : public Expr
	{
		RefExpr(Token *tagToken);
		RefExpr(Token *tagToken, Expr *refExpr);
		~RefExpr() override;
		std::wstring ToString() override;

		Expr *refExpr;
	};

	struct LambdaExpr : public Expr
	{
		LambdaExpr(Token *tagToken);
		LambdaExpr(Token *tagToken, const std::vector<VarDescExpr *> &parameters, struct ScopeStmt *body);
		~LambdaExpr() override;
		std::wstring ToString() override;

		std::vector<VarDescExpr *> parameters;
		struct ScopeStmt *body;
	};

	struct CallExpr : public Expr
	{
		CallExpr(Token *tagToken);
		CallExpr(Token *tagToken, Expr *callee, const std::vector<Expr *> &arguments);
		~CallExpr() override;
		std::wstring ToString() override;

		Expr *callee;
		std::vector<Expr *> arguments;
	};

	struct DotExpr : public Expr
	{
		DotExpr(Token *tagToken);
		DotExpr(Token *tagToken, Expr *callee, IdentifierExpr *callMember);
		~DotExpr() override;
		std::wstring ToString() override;

		Expr *callee;
		IdentifierExpr *callMember;
	};

	struct NewExpr : public Expr
	{
		NewExpr(Token *tagToken);
		NewExpr(Token *tagToken, Expr *callee);
		~NewExpr() override;
		std::wstring ToString() override;

		Expr *callee;
	};

	AST_TEMPLATE(ThisExpr, Expr)

	struct BaseExpr : public Expr
	{
		BaseExpr(Token *tagToken, IdentifierExpr *callMember);
		~BaseExpr() override;
		std::wstring ToString() override;

		IdentifierExpr *callMember;
	};

	struct CompoundExpr : public Expr
	{
		CompoundExpr(Token *tagToken);
		CompoundExpr(Token *tagToken, const std::vector<struct Stmt *> &stmts, Expr *endExpr);
		~CompoundExpr() override;
		std::wstring ToString() override;

		std::vector<struct Stmt *> stmts;
		Expr *endExpr;
	};

	struct AnonyObjExpr : public Expr
	{
		AnonyObjExpr(Token *tagToken);
		AnonyObjExpr(Token *tagToken, const std::vector<std::pair<std::wstring, Expr *>> &elements);
		~AnonyObjExpr() override;
		std::wstring ToString() override;

		std::vector<std::pair<std::wstring, Expr *>> elements;
	};

	struct VarArgExpr : public Expr
	{
		VarArgExpr(Token *tagToken);
		VarArgExpr(Token *tagToken, IdentifierExpr *argName);
		~VarArgExpr() override;
		std::wstring ToString() override;
		IdentifierExpr *argName;
	};

	struct FactorialExpr : public Expr
	{
		FactorialExpr(Token *tagToken);
		FactorialExpr(Token *tagToken, Expr *expr);
		~FactorialExpr() override;
		std::wstring ToString() override;
		Expr *expr;
	};

	struct AppregateExpr : public Expr
	{
		AppregateExpr(Token *tagToken);
		AppregateExpr(Token *tagToken, const std::vector<Expr *> &exprs);
		~AppregateExpr() override;
		std::wstring ToString() override;
		std::vector<Expr *> exprs;
	};

	struct Stmt : public AstNode
	{
		Stmt(Token *tagToken, AstKind kind) : AstNode(tagToken, kind) {}
		virtual ~Stmt() {}
		virtual std::wstring ToString() = 0;
	};

	struct ExprStmt : public Stmt
	{
		ExprStmt(Token *tagToken);
		ExprStmt(Token *tagToken, Expr *expr);
		~ExprStmt() override;
		std::wstring ToString() override;

		Expr *expr;
	};

	struct VarStmt : public Stmt
	{
		VarStmt(Token *tagToken);
		VarStmt(Token *tagToken, Privilege privilege, const std::vector<std::pair<Expr *, Expr *>> &variables);
		~VarStmt() override;
		std::wstring ToString() override;

		Privilege privilege;
		std::vector<std::pair<Expr *, Expr *>> variables;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt(Token *tagToken);
		ReturnStmt(Token *tagToken, Expr *expr);
		~ReturnStmt() override;
		std::wstring ToString() override;

		Expr *expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt(Token *tagToken);
		IfStmt(Token *tagToken, Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
		~IfStmt() override;
		std::wstring ToString() override;

		Expr *condition;
		Stmt *thenBranch;
		Stmt *elseBranch;
	};

	struct ScopeStmt : public Stmt
	{
		ScopeStmt(Token *tagToken);
		ScopeStmt(Token *tagToken, const std::vector<Stmt *> &stmts);
		~ScopeStmt() override;
		std::wstring ToString() override;

		std::vector<Stmt *> stmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt(Token *tagToken);
		WhileStmt(Token *tagToken, Expr *condition, ScopeStmt *body, ScopeStmt *increment = nullptr);
		~WhileStmt() override;
		std::wstring ToString() override;

		Expr *condition;
		ScopeStmt *body;
		ScopeStmt *increment;
	};

	AST_TEMPLATE(BreakStmt, Stmt)
	AST_TEMPLATE(ContinueStmt, Stmt)

	struct EnumStmt : public Stmt
	{
		EnumStmt(Token *tagToken);
		EnumStmt(Token *tagToken, IdentifierExpr *name, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems);
		~EnumStmt() override;

		std::wstring ToString() override;

		IdentifierExpr *name;
		std::unordered_map<IdentifierExpr *, Expr *> enumItems;
	};

	enum class FunctionKind
	{
		CLASS_CONSTRUCTOR,
		CLASS_CLOSURE,
		FUNCTION,
	};

	struct FunctionStmt : public Stmt
	{
		FunctionStmt(Token *tagToken);
		FunctionStmt(Token *tagToken, FunctionKind kind, IdentifierExpr *name, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body);
		~FunctionStmt() override;

		std::wstring ToString() override;

		FunctionKind functionKind;
		IdentifierExpr *name;
		std::vector<VarDescExpr *> parameters;
		ScopeStmt *body;
	};

	struct ClassStmt : public Stmt
	{
		ClassStmt(Token *tagToken);
		ClassStmt(Token *tagToken,
				  std::wstring name,
				  const std::vector<VarStmt *> &varItems,
				  const std::vector<FunctionStmt *> &fnItems,
				  const std::vector<EnumStmt *> &enumItems,
				  const std::vector<FunctionStmt *> &constructors = {},
				  const std::vector<IdentifierExpr *> &parentClasses = {});
		~ClassStmt() override;

		std::wstring ToString() override;

		std::wstring name;
		std::vector<IdentifierExpr *> parentClasses;
		std::vector<FunctionStmt *> constructors;
		std::vector<VarStmt *> varItems;
		std::vector<FunctionStmt *> fnItems;
		std::vector<EnumStmt *> enumItems;
	};

	struct ModuleStmt : public Stmt
	{
		ModuleStmt(Token *tagToken);
		ModuleStmt(Token *tagToken,
				   IdentifierExpr *name,
				   const std::vector<VarStmt *> &varItems,
				   const std::vector<ClassStmt *> &classItems,
				   const std::vector<ModuleStmt *> &moduleItems,
				   const std::vector<EnumStmt *> &enumItems,
				   const std::vector<FunctionStmt *> &functionItems);
		~ModuleStmt() override;

		std::wstring ToString() override;

		IdentifierExpr *name;
		std::vector<VarStmt *> varItems;
		std::vector<ClassStmt *> classItems;
		std::vector<ModuleStmt *> moduleItems;
		std::vector<EnumStmt *> enumItems;
		std::vector<FunctionStmt *> functionItems;
	};
	struct AstStmts : public Stmt
	{
		AstStmts(Token *tagToken);
		AstStmts(Token *tagToken, std::vector<Stmt *> stmts);
		~AstStmts() override;

		std::wstring ToString() override;

		std::vector<Stmt *> stmts;
	};
}