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
		AST_GROUP,
		AST_ARRAY,
		AST_TABLE,
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
		// stmt
		AST_LET,
		AST_CONST,
		AST_EXPR,
		AST_RETURN,
		AST_IF,
		AST_SCOPE,
		AST_WHILE,
		AST_BREAK,
		AST_CONTINUE,
		AST_ENUM,
		AST_FUNCTION,
		AST_CLASS,
		AST_ASTSTMTS,
	};

	struct AstNode
	{
		AstNode() {}
		virtual ~AstNode() {}

		virtual std::wstring Stringify() = 0;
		virtual AstType Type() const = 0;

		uint64_t line{1};
		uint64_t column{1};
	};

	struct Expr : public AstNode
	{
		Expr() {}
		virtual ~Expr() {}

		virtual std::wstring Stringify() = 0;
		virtual AstType Type() const = 0;
	};

	struct IntNumExpr : public Expr
	{
		IntNumExpr();
		IntNumExpr(int64_t value);
		~IntNumExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		int64_t value;
	};

	struct RealNumExpr : public Expr
	{
		RealNumExpr();
		RealNumExpr(double value);
		~RealNumExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		double value;
	};

	struct StrExpr : public Expr
	{
		StrExpr();
		StrExpr(std::wstring_view str);
		~StrExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::wstring value;
	};

	struct NullExpr : public Expr
	{
		NullExpr();
		~NullExpr();

		std::wstring Stringify() override;
		AstType Type() const override;
	};

	struct BoolExpr : public Expr
	{
		BoolExpr();
		BoolExpr(bool value);
		~BoolExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		bool value;
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr();
		IdentifierExpr(std::wstring_view literal);
		~IdentifierExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::wstring literal;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr();
		ArrayExpr(std::vector<Expr *> elements);
		~ArrayExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::vector<Expr *> elements;
	};

	struct TableExpr : public Expr
	{
		TableExpr();
		TableExpr(const std::vector<std::pair<Expr*, Expr*>>& elements);
		~TableExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::vector<std::pair<Expr *, Expr *>> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr();
		GroupExpr(Expr *expr);
		~GroupExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr();
		PrefixExpr(std::wstring_view op, Expr *right);
		~PrefixExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::wstring op;
		Expr *right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr();
		InfixExpr(std::wstring_view op, Expr *left, Expr *right);
		~InfixExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::wstring op;
		Expr *left;
		Expr *right;
	};

	struct PostfixExpr : public Expr
	{
		PostfixExpr();
		PostfixExpr(Expr *left, std::wstring_view op);
		~PostfixExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *left;
		std::wstring op;
	};

	struct ConditionExpr : public Expr
	{
		ConditionExpr();
		ConditionExpr(Expr *condition, Expr *trueBranch, Expr *falseBranch);
		~ConditionExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *condition;
		Expr *trueBranch;
		Expr *falseBranch;
	};

	struct IndexExpr : public Expr
	{
		IndexExpr();
		IndexExpr(Expr *ds, Expr *index);
		~IndexExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *ds;
		Expr *index;
	};

	struct RefExpr : public Expr
	{
		RefExpr();
		RefExpr(Expr *refExpr);
		~RefExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *refExpr;
	};

	struct LambdaExpr : public Expr
	{
		LambdaExpr();
		LambdaExpr(std::vector<IdentifierExpr *> parameters, struct ScopeStmt *body);
		~LambdaExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::vector<IdentifierExpr *> parameters;
		struct ScopeStmt *body;
	};

	struct CallExpr : public Expr
	{
		CallExpr();
		CallExpr(Expr *callee, std::vector<Expr *> arguments);
		~CallExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *callee;
		std::vector<Expr *> arguments;
	};

	struct DotExpr : public Expr
	{
		DotExpr();
		DotExpr(Expr *callee, IdentifierExpr *callMember);
		~DotExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *callee;
		IdentifierExpr *callMember;
	};

	struct NewExpr : public Expr
	{
		NewExpr();
		NewExpr(CallExpr *callee);
		~NewExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		CallExpr *callee;
	};

	struct ThisExpr : public Expr
	{
		ThisExpr();
		~ThisExpr();

		std::wstring Stringify() override;
		AstType Type() const override;
	};

	struct BaseExpr : public Expr
	{
		BaseExpr(IdentifierExpr *callMember);
		~BaseExpr();

		std::wstring Stringify() override;
		AstType Type() const override;

		IdentifierExpr *callMember;
	};

	struct Stmt : public AstNode
	{
		Stmt() {}
		virtual ~Stmt() {}

		virtual std::wstring Stringify() = 0;
		virtual AstType Type() const = 0;
	};

	struct ExprStmt : public Stmt
	{
		ExprStmt();
		ExprStmt(Expr *expr);
		~ExprStmt();
		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *expr;
	};

	struct VarDesc
	{
		std::wstring type;
		Expr *value;
	};

	struct LetStmt : public Stmt
	{
		LetStmt();
		LetStmt(const std::unordered_map<IdentifierExpr *, VarDesc> &variables);
		~LetStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::unordered_map<IdentifierExpr *, VarDesc> variables;
	};

	struct ConstStmt : public Stmt
	{
		ConstStmt();
		ConstStmt(const std::unordered_map<IdentifierExpr *, VarDesc> &consts);
		~ConstStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::unordered_map<IdentifierExpr *, VarDesc> consts;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt();
		ReturnStmt(Expr *expr);
		~ReturnStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt();
		IfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
		~IfStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *condition;
		Stmt *thenBranch;
		Stmt *elseBranch;
	};

	struct ScopeStmt : public Stmt
	{
		ScopeStmt();
		ScopeStmt(std::vector<Stmt *> stmts);
		~ScopeStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::vector<Stmt *> stmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt();
		WhileStmt(Expr *condition, ScopeStmt *body, ScopeStmt *increment = nullptr);
		~WhileStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		Expr *condition;
		ScopeStmt *body;
		ScopeStmt *increment;
	};

	struct BreakStmt : public Stmt
	{
		BreakStmt();
		~BreakStmt();

		std::wstring Stringify() override;
		AstType Type() const override;
	};

	struct ContinueStmt : public Stmt
	{
		ContinueStmt();
		~ContinueStmt();

		std::wstring Stringify() override;
		AstType Type() const override;
	};

	struct EnumStmt : public Stmt
	{
		EnumStmt();
		EnumStmt(IdentifierExpr *enumName, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems);
		~EnumStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		IdentifierExpr *enumName;
		std::unordered_map<IdentifierExpr *, Expr *> enumItems;
	};

	enum class FunctionType
	{
		CLASS_CONSTRUCTOR,
		CLASS_CLOSURE,
		NORMAL_FUNCTION,
	};

	struct FunctionStmt : public Stmt
	{
		FunctionStmt();
		FunctionStmt(FunctionType type, IdentifierExpr *name, std::vector<IdentifierExpr *> parameters, ScopeStmt *body);
		~FunctionStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		FunctionType type;
		IdentifierExpr *name;
		std::vector<IdentifierExpr *> parameters;
		ScopeStmt *body;
	};

	struct ClassStmt : public Stmt
	{
		ClassStmt();
		ClassStmt(std::wstring name,
				  std::vector<LetStmt *> letStmts,
				  std::vector<ConstStmt *> constStmts,
				  std::vector<FunctionStmt *> fnStmts,
				  std::vector<FunctionStmt*> constructors={},
				  std::vector<IdentifierExpr *> parentClasses = {});
		~ClassStmt();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::wstring name;
		std::vector<IdentifierExpr *> parentClasses;
		std::vector<FunctionStmt*> constructors;
		std::vector<LetStmt *> letStmts;
		std::vector<ConstStmt *> constStmts;
		std::vector<FunctionStmt *> fnStmts;
	};

	struct AstStmts : public Stmt
	{
		AstStmts();
		AstStmts(std::vector<Stmt *> stmts);
		~AstStmts();

		std::wstring Stringify() override;
		AstType Type() const override;

		std::vector<Stmt *> stmts;
	};
}