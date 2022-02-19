#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
namespace lws
{
	enum class AstType
	{
		// expr
		INT,
		REAL,
		STR,
		NIL,
		BOOL,
		NOT,
		IDENTIFIER,
		GROUP,
		ARRAY,
		TABLE,
		PREFIX,
		INFIX,
		CONDITION,
		INDEX,
		REF,
		LAMBDA,
		FUNCTION_CALL,
		FIELD_CALL,
		NEW,
		// stmt
		LET,
		EXPR,
		RETURN,
		IF,
		SCOPE,
		WHILE,
		FUNCTION,
		FIELD,
		ASTSTMTS,
	};

	struct AstNode
	{
		AstNode() {}
		virtual ~AstNode() {}

		virtual std::string Stringify() = 0;
		virtual AstType Type() = 0;
	};

	struct Expr
	{
		Expr() {}
		virtual ~Expr() {}

		virtual std::string Stringify() = 0;
		virtual AstType Type() = 0;
	};

	struct IntNumExpr : public Expr
	{
		IntNumExpr();
		IntNumExpr(int64_t value);
		~IntNumExpr();

		std::string Stringify() override;
		AstType Type() override;

		int64_t value;
	};

	struct RealNumExpr : public Expr
	{
		RealNumExpr();
		RealNumExpr(double value);
		~RealNumExpr();

		std::string Stringify() override;
		AstType Type() override;

		double value;
	};

	struct StrExpr : public Expr
	{
		StrExpr();
		StrExpr(std::string_view str);
		~StrExpr();

		std::string Stringify() override;
		AstType Type() override;

		std::string value;
	};

	struct NilExpr : public Expr
	{
		NilExpr();
		~NilExpr();

		std::string Stringify() override;
		AstType Type() override;
	};

	struct BoolExpr : public Expr
	{
		BoolExpr();
		BoolExpr(bool value);
		~BoolExpr();

		std::string Stringify() override;
		AstType Type() override;
		bool value;
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr();
		IdentifierExpr(std::string_view literal);
		~IdentifierExpr();

		std::string Stringify() override;
		AstType Type() override;

		std::string literal;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr();
		ArrayExpr(std::vector<Expr *> elements);
		~ArrayExpr();

		std::string Stringify() override;
		AstType Type() override;

		std::vector<Expr *> elements;
	};

	struct TableExpr : public Expr
	{
		TableExpr();
		TableExpr(std::unordered_map<Expr *, Expr *> elements);
		~TableExpr();

		std::string Stringify() override;
		AstType Type() override;

		std::unordered_map<Expr *, Expr *> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr();
		GroupExpr(Expr *expr);
		~GroupExpr();

		std::string Stringify() override;
		AstType Type() override;

		Expr *expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr();
		PrefixExpr(std::string_view op, Expr *right);
		~PrefixExpr();

		std::string Stringify() override;
		AstType Type() override;

		std::string op;
		Expr *right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr();
		InfixExpr(std::string_view op, Expr *left, Expr *right);
		~InfixExpr();

		std::string Stringify() override;
		AstType Type() override;

		std::string op;
		Expr *left;
		Expr *right;
	};

	struct ConditionExpr : public Expr
	{
		ConditionExpr();
		ConditionExpr(Expr *condition, Expr *left, Expr *right);
		~ConditionExpr();

		std::string Stringify() override;
		AstType Type() override;

		Expr *condition;
		Expr *trueBranch;
		Expr *falseBranch;
	};

	struct IndexExpr : public Expr
	{
		IndexExpr();
		IndexExpr(Expr *ds, Expr *index);
		~IndexExpr();
		std::string Stringify() override;

		AstType Type() override;

		Expr *ds;
		Expr *index;
	};

	struct RefExpr : public Expr
	{
		RefExpr();
		RefExpr(Expr *refExpr);
		~RefExpr();

		std::string Stringify() override;
		AstType Type() override;

		Expr *refExpr;
	};

	struct LambdaExpr : public Expr
	{
		LambdaExpr();
		LambdaExpr(std::vector<IdentifierExpr *> parameters, struct ScopeStmt *body);
		~LambdaExpr();
		std::string Stringify() override;
		AstType Type() override;

		std::vector<IdentifierExpr *> parameters;
		struct ScopeStmt *body;
	};

	struct FunctionCallExpr : public Expr
	{
		FunctionCallExpr();
		FunctionCallExpr(Expr *name, std::vector<Expr *> arguments);
		~FunctionCallExpr();

		std::string Stringify() override;
		AstType Type() override;

		Expr *name;
		std::vector<Expr *> arguments;
	};

	struct FieldCallExpr : public Expr
	{
		FieldCallExpr();
		FieldCallExpr(Expr *callee, Expr *callMember);
		~FieldCallExpr();

		std::string Stringify() override;
		AstType Type() override;

		Expr *callee;
		Expr *callMember;
	};

	struct Stmt : public AstNode
	{
		Stmt() {}
		virtual ~Stmt() {}

		virtual std::string Stringify() = 0;
		virtual AstType Type() = 0;
	};

	struct ExprStmt : public Stmt
	{
		ExprStmt();
		ExprStmt(Expr *expr);
		~ExprStmt();
		std::string Stringify() override;
		AstType Type() override;

		Expr *expr;
	};

	struct LetStmt : public Stmt
	{
		LetStmt();
		LetStmt(const std::unordered_map<IdentifierExpr *, Expr *> &variables);
		~LetStmt();

		std::string Stringify() override;
		AstType Type() override;

		std::unordered_map<IdentifierExpr *, Expr *> variables;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt();
		ReturnStmt(Expr *expr);
		~ReturnStmt();

		std::string Stringify() override;
		AstType Type() override;

		Expr *expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt();
		IfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
		~IfStmt();

		std::string Stringify() override;
		AstType Type() override;
		Expr *condition;
		Stmt *thenBranch;
		Stmt *elseBranch;
	};

	struct ScopeStmt : public Stmt
	{
		ScopeStmt();
		ScopeStmt(std::vector<Stmt *> stmts);
		~ScopeStmt();

		std::string Stringify() override;
		AstType Type() override;
		std::vector<Stmt *> stmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt();
		WhileStmt(Expr *condition, Stmt *body);
		~WhileStmt();

		std::string Stringify() override;
		AstType Type() override;

		Expr *condition;
		Stmt *body;
	};

	struct FunctionStmt : public Stmt
	{
		FunctionStmt();
		FunctionStmt(IdentifierExpr *name, std::vector<IdentifierExpr *> parameters, ScopeStmt *body);
		~FunctionStmt();

		std::string Stringify() override;
		AstType Type() override;

		IdentifierExpr *name;
		std::vector<IdentifierExpr *> parameters;
		ScopeStmt *body;
	};

	struct FieldStmt : public Stmt
	{
		FieldStmt();
		FieldStmt(std::string name,
				  std::vector<LetStmt *> letStmts,
				  std::vector<FunctionStmt *> fnStmts,
				  std::vector<IdentifierExpr *> containedFields = {});
		~FieldStmt();

		std::string Stringify() override;
		AstType Type() override;

		std::string name;
		std::vector<IdentifierExpr *> containedFields;
		std::vector<LetStmt *> letStmts;
		std::vector<FunctionStmt *> fnStmts;
	};

	struct AstStmts : public Stmt
	{
		AstStmts();
		AstStmts(std::vector<Stmt *> stmts);
		~AstStmts();

		std::string Stringify() override;
		AstType Type() override;

		std::vector<Stmt *> stmts;
	};
}