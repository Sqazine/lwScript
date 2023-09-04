#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Token.h"
namespace lws
{
	enum AstType
	{
		// expr
		AST_INT,
		AST_REAL,
		AST_STR,
		AST_NULL,
		AST_BOOL,
		AST_IDENTIFIER,
		AST_VAR_DESC,
		AST_GROUP,
		AST_ARRAY,
		AST_DICT,
		AST_PREFIX,
		AST_INFIX,
		AST_POSTFIX,
		AST_CONDITION,
		AST_INDEX,
		AST_REF,
		AST_LAMBDA,
		AST_DOT,
		AST_CALL,
		AST_NEW,
		AST_THIS,
		AST_BASE,
		AST_BLOCK,
		AST_ANONY_OBJ,
		AST_VAR_ARG,
		AST_FACTORIAL,
		AST_APPREGATE,
		// stmt
		AST_VAR,
		AST_EXPR,
		AST_RETURN,
		AST_IF,
		AST_SCOPE,
		AST_WHILE,
		AST_BREAK,
		AST_CONTINUE,
		AST_ENUM,
		AST_MODULE,
		AST_FUNCTION,
		AST_CLASS,
		AST_ASTSTMTS,
	};

	struct AstNode
	{
		AstNode(Token tagToken, AstType type) : tagToken(tagToken), type(type) {}
		virtual ~AstNode() {}

		virtual std::wstring ToString() = 0;

		const AstType type;

		Token tagToken;
	};

	struct Expr : public AstNode
	{
		Expr(Token tagToken, AstType type) : AstNode(tagToken, type) {}
		virtual ~Expr() {}

		virtual std::wstring ToString() = 0;
	};

	struct IntNumExpr : public Expr
	{
		IntNumExpr(Token tagToken);
		IntNumExpr(Token tagToken, int64_t value);
		~IntNumExpr();

		std::wstring ToString() override;

		int64_t value;
	};

	struct RealNumExpr : public Expr
	{
		RealNumExpr(Token tagToken);
		RealNumExpr(Token tagToken, double value);
		~RealNumExpr();

		std::wstring ToString() override;

		double value;
	};

	struct StrExpr : public Expr
	{
		StrExpr(Token tagToken);
		StrExpr(Token tagToken, std::wstring_view str);
		~StrExpr();

		std::wstring ToString() override;

		std::wstring value;
	};

	struct NullExpr : public Expr
	{
		NullExpr(Token tagToken);
		~NullExpr();

		std::wstring ToString() override;
	};

	struct BoolExpr : public Expr
	{
		BoolExpr(Token tagToken);
		BoolExpr(Token tagToken, bool value);
		~BoolExpr();

		std::wstring ToString() override;

		bool value;
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr(Token tagToken);
		IdentifierExpr(Token tagToken, std::wstring_view literal);
		~IdentifierExpr();

		std::wstring ToString() override;

		std::wstring literal;
	};

	struct VarDescExpr : public Expr
	{
		VarDescExpr(Token tagToken);
		VarDescExpr(Token tagToken, std::wstring_view typeDesc, Expr *name);
		~VarDescExpr();

		std::wstring ToString() override;

		std::wstring typeDesc;
		Expr *name;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr(Token tagToken);
		ArrayExpr(Token tagToken, const std::vector<Expr *> &elements);
		~ArrayExpr();

		std::wstring ToString() override;

		std::vector<Expr *> elements;
	};

	struct DictExpr : public Expr
	{
		DictExpr(Token tagToken);
		DictExpr(Token tagToken, const std::vector<std::pair<Expr *, Expr *>> &elements);
		~DictExpr();

		std::wstring ToString() override;

		std::vector<std::pair<Expr *, Expr *>> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr(Token tagToken);
		GroupExpr(Token tagToken, Expr *expr);
		~GroupExpr();

		std::wstring ToString() override;

		Expr *expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr(Token tagToken);
		PrefixExpr(Token tagToken, std::wstring_view op, Expr *right);
		~PrefixExpr();

		std::wstring ToString() override;

		std::wstring op;
		Expr *right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr(Token tagToken);
		InfixExpr(Token tagToken, std::wstring_view op, Expr *left, Expr *right);
		~InfixExpr();

		std::wstring ToString() override;

		std::wstring op;
		Expr *left;
		Expr *right;
	};

	struct PostfixExpr : public Expr
	{
		PostfixExpr(Token tagToken);
		PostfixExpr(Token tagToken, Expr *left, std::wstring_view op);
		~PostfixExpr();

		std::wstring ToString() override;

		Expr *left;
		std::wstring op;
	};

	struct ConditionExpr : public Expr
	{
		ConditionExpr(Token tagToken);
		ConditionExpr(Token tagToken, Expr *condition, Expr *trueBranch, Expr *falseBranch);
		~ConditionExpr();

		std::wstring ToString() override;

		Expr *condition;
		Expr *trueBranch;
		Expr *falseBranch;
	};

	struct IndexExpr : public Expr
	{
		IndexExpr(Token tagToken);
		IndexExpr(Token tagToken, Expr *ds, Expr *index);
		~IndexExpr();

		std::wstring ToString() override;

		Expr *ds;
		Expr *index;
	};

	struct RefExpr : public Expr
	{
		RefExpr(Token tagToken);
		RefExpr(Token tagToken, Expr *refExpr);
		~RefExpr();

		std::wstring ToString() override;

		Expr *refExpr;
	};

	struct LambdaExpr : public Expr
	{
		LambdaExpr(Token tagToken);
		LambdaExpr(Token tagToken, const std::vector<VarDescExpr *> &parameters, struct ScopeStmt *body);
		~LambdaExpr();

		std::wstring ToString() override;

		std::vector<VarDescExpr *> parameters;
		struct ScopeStmt *body;
	};

	struct CallExpr : public Expr
	{
		CallExpr(Token tagToken);
		CallExpr(Token tagToken, Expr *callee, const std::vector<Expr *> &arguments);
		~CallExpr();

		std::wstring ToString() override;

		Expr *callee;
		std::vector<Expr *> arguments;
	};

	struct DotExpr : public Expr
	{
		DotExpr(Token tagToken);
		DotExpr(Token tagToken, Expr *callee, IdentifierExpr *callMember);
		~DotExpr();

		std::wstring ToString() override;

		Expr *callee;
		IdentifierExpr *callMember;
	};

	struct NewExpr : public Expr
	{
		NewExpr(Token tagToken);
		NewExpr(Token tagToken, Expr *callee);
		~NewExpr();

		std::wstring ToString() override;

		Expr *callee;
	};

	struct ThisExpr : public Expr
	{
		ThisExpr(Token tagToken);
		~ThisExpr();

		std::wstring ToString() override;
	};

	struct BaseExpr : public Expr
	{
		BaseExpr(Token tagToken, IdentifierExpr *callMember);
		~BaseExpr();

		std::wstring ToString() override;

		IdentifierExpr *callMember;
	};

	struct BlockExpr : public Expr
	{
		BlockExpr(Token tagToken);
		BlockExpr(Token tagToken, const std::vector<struct Stmt *> &stmts, Expr *endExpr);
		~BlockExpr();

		std::wstring ToString() override;

		std::vector<struct Stmt *> stmts;
		Expr *endExpr;
	};

	struct AnonyObjExpr : public Expr
	{
		AnonyObjExpr(Token tagToken);
		AnonyObjExpr(Token tagToken, const std::vector<std::pair<std::wstring, Expr *>> &elements);
		~AnonyObjExpr();

		std::wstring ToString() override;

		std::vector<std::pair<std::wstring, Expr *>> elements;
	};

	struct VarArgExpr : public Expr
	{
		VarArgExpr(Token tagToken);
		VarArgExpr(Token tagToken, IdentifierExpr *argName);
		~VarArgExpr();

		std::wstring ToString() override;
		IdentifierExpr *argName;
	};

	struct FactorialExpr : public Expr
	{
		FactorialExpr(Token tagToken);
		FactorialExpr(Token tagToken, Expr *expr);
		~FactorialExpr();

		std::wstring ToString() override;

		Expr *expr;
	};

	struct AppregateExpr : public Expr
	{
		AppregateExpr(Token tagToken);
		AppregateExpr(Token tagToken, const std::vector<Expr *> &exprs);
		~AppregateExpr();

		std::wstring ToString() override;

		std::vector<Expr *> exprs;
	};

	struct Stmt : public AstNode
	{
		Stmt(Token tagToken, AstType type) : AstNode(tagToken, type) {}
		virtual ~Stmt() {}

		virtual std::wstring ToString() = 0;
	};

	struct ExprStmt : public Stmt
	{
		ExprStmt(Token tagToken);
		ExprStmt(Token tagToken, Expr *expr);
		~ExprStmt();
		std::wstring ToString() override;

		Expr *expr;
	};

	struct VarStmt : public Stmt
	{
		enum class Privilege
		{
			MUTABLE,
			IMMUTABLE,
		};

		VarStmt(Token tagToken);
		VarStmt(Token tagToken, Privilege privilege, const std::vector<std::pair<Expr *, Expr *>> &variables);
		~VarStmt();

		std::wstring ToString() override;

		Privilege privilege;
		std::vector<std::pair<Expr *, Expr *>> variables;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt(Token tagToken);
		ReturnStmt(Token tagToken, Expr *expr);
		~ReturnStmt();

		std::wstring ToString() override;

		Expr *expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt(Token tagToken);
		IfStmt(Token tagToken, Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
		~IfStmt();

		std::wstring ToString() override;

		Expr *condition;
		Stmt *thenBranch;
		Stmt *elseBranch;
	};

	struct ScopeStmt : public Stmt
	{
		ScopeStmt(Token tagToken);
		ScopeStmt(Token tagToken, const std::vector<Stmt *> &stmts);
		~ScopeStmt();

		std::wstring ToString() override;

		std::vector<Stmt *> stmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt(Token tagToken);
		WhileStmt(Token tagToken, Expr *condition, ScopeStmt *body, ScopeStmt *increment = nullptr);
		~WhileStmt();

		std::wstring ToString() override;

		Expr *condition;
		ScopeStmt *body;
		ScopeStmt *increment;
	};

	struct BreakStmt : public Stmt
	{
		BreakStmt(Token tagToken);
		~BreakStmt();

		std::wstring ToString() override;
	};

	struct ContinueStmt : public Stmt
	{
		ContinueStmt(Token tagToken);
		~ContinueStmt();

		std::wstring ToString() override;
	};

	struct EnumStmt : public Stmt
	{
		EnumStmt(Token tagToken);
		EnumStmt(Token tagToken, IdentifierExpr *enumName, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems);
		~EnumStmt();

		std::wstring ToString() override;

		IdentifierExpr *enumName;
		std::unordered_map<IdentifierExpr *, Expr *> enumItems;
	};

	struct ModuleStmt : public Stmt
	{
		ModuleStmt(Token tagToken);
		ModuleStmt(Token tagToken, IdentifierExpr *modName, const std::vector<Stmt *> &modItems);
		~ModuleStmt();

		std::wstring ToString() override;

		IdentifierExpr *modName;
		std::vector<Stmt *> modItems;
	};

	enum class FunctionType
	{
		CLASS_CONSTRUCTOR,
		CLASS_CLOSURE,
		FUNCTION,
	};

	struct FunctionStmt : public Stmt
	{
		FunctionStmt(Token tagToken);
		FunctionStmt(Token tagToken, FunctionType type, IdentifierExpr *name, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body);
		~FunctionStmt();

		std::wstring ToString() override;

		FunctionType type;
		IdentifierExpr *name;
		std::vector<VarDescExpr *> parameters;
		ScopeStmt *body;
	};

	struct ClassStmt : public Stmt
	{
		ClassStmt(Token tagToken);
		ClassStmt(Token tagToken,
				  std::wstring name,
				  std::vector<VarStmt *> varStmts,
				  std::vector<FunctionStmt *> fnStmts,
				  std::vector<FunctionStmt *> constructors = {},
				  std::vector<IdentifierExpr *> parentClasses = {});
		~ClassStmt();

		std::wstring ToString() override;

		std::wstring name;
		std::vector<IdentifierExpr *> parentClasses;
		std::vector<FunctionStmt *> constructors;
		std::vector<VarStmt *> varStmts;
		std::vector<FunctionStmt *> fnStmts;
	};

	struct AstStmts : public Stmt
	{
		AstStmts(Token tagToken);
		AstStmts(Token tagToken, std::vector<Stmt *> stmts);
		~AstStmts();

		std::wstring ToString() override;

		std::vector<Stmt *> stmts;
	};
}