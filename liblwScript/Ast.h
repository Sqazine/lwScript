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
		AstNode(AstType type) : type(type) {}
		virtual ~AstNode() {}

		virtual std::wstring Stringify() = 0;

		const AstType type;

		uint64_t line{1};
		uint64_t column{1};
	};

	struct Expr : public AstNode
	{
		Expr(AstType type) : AstNode(type) {}
		virtual ~Expr() {}

		virtual std::wstring Stringify() = 0;
	};

	struct IntNumExpr : public Expr
	{
		IntNumExpr();
		IntNumExpr(int64_t value);
		~IntNumExpr();

		std::wstring Stringify() override;

		int64_t value;
	};

	struct RealNumExpr : public Expr
	{
		RealNumExpr();
		RealNumExpr(double value);
		~RealNumExpr();

		std::wstring Stringify() override;

		double value;
	};

	struct StrExpr : public Expr
	{
		StrExpr();
		StrExpr(std::wstring_view str);
		~StrExpr();

		std::wstring Stringify() override;

		std::wstring value;
	};

	struct NullExpr : public Expr
	{
		NullExpr();
		~NullExpr();

		std::wstring Stringify() override;
	};

	struct BoolExpr : public Expr
	{
		BoolExpr();
		BoolExpr(bool value);
		~BoolExpr();

		std::wstring Stringify() override;

		bool value;
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr();
		IdentifierExpr(std::wstring_view literal);
		~IdentifierExpr();

		std::wstring Stringify() override;

		std::wstring literal;
	};

	struct VarDescExpr : public Expr
	{
		VarDescExpr();
		VarDescExpr(std::wstring_view typeDesc, Expr *name);
		~VarDescExpr();

		std::wstring Stringify() override;

		std::wstring typeDesc;
		Expr *name;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr();
		ArrayExpr(std::vector<Expr *> elements);
		~ArrayExpr();

		std::wstring Stringify() override;

		std::vector<Expr *> elements;
	};

	struct DictExpr : public Expr
	{
		DictExpr();
		DictExpr(const std::vector<std::pair<Expr *, Expr *>> &elements);
		~DictExpr();

		std::wstring Stringify() override;

		std::vector<std::pair<Expr *, Expr *>> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr();
		GroupExpr(Expr *expr);
		~GroupExpr();

		std::wstring Stringify() override;

		Expr *expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr();
		PrefixExpr(std::wstring_view op, Expr *right);
		~PrefixExpr();

		std::wstring Stringify() override;

		std::wstring op;
		Expr *right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr();
		InfixExpr(std::wstring_view op, Expr *left, Expr *right);
		~InfixExpr();

		std::wstring Stringify() override;

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

		Expr *left;
		std::wstring op;
	};

	struct ConditionExpr : public Expr
	{
		ConditionExpr();
		ConditionExpr(Expr *condition, Expr *trueBranch, Expr *falseBranch);
		~ConditionExpr();

		std::wstring Stringify() override;

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

		Expr *ds;
		Expr *index;
	};

	struct RefExpr : public Expr
	{
		RefExpr();
		RefExpr(Expr *refExpr);
		~RefExpr();

		std::wstring Stringify() override;

		Expr *refExpr;
	};

	struct LambdaExpr : public Expr
	{
		LambdaExpr();
		LambdaExpr(std::vector<IdentifierExpr *> parameters, struct ScopeStmt *body);
		~LambdaExpr();

		std::wstring Stringify() override;

		std::vector<IdentifierExpr *> parameters;
		struct ScopeStmt *body;
	};

	struct CallExpr : public Expr
	{
		CallExpr();
		CallExpr(Expr *callee, std::vector<Expr *> arguments);
		~CallExpr();

		std::wstring Stringify() override;

		Expr *callee;
		std::vector<Expr *> arguments;
	};

	struct DotExpr : public Expr
	{
		DotExpr();
		DotExpr(Expr *callee, IdentifierExpr *callMember);
		~DotExpr();

		std::wstring Stringify() override;

		Expr *callee;
		IdentifierExpr *callMember;
	};

	struct NewExpr : public Expr
	{
		NewExpr();
		NewExpr(Expr *callee);
		~NewExpr();

		std::wstring Stringify() override;

		Expr *callee;
	};

	struct ThisExpr : public Expr
	{
		ThisExpr();
		~ThisExpr();

		std::wstring Stringify() override;
	};

	struct BaseExpr : public Expr
	{
		BaseExpr(IdentifierExpr *callMember);
		~BaseExpr();

		std::wstring Stringify() override;

		IdentifierExpr *callMember;
	};

	struct BlockExpr : public Expr
	{
		BlockExpr();
		BlockExpr(const std::vector<struct Stmt *> &stmts, Expr *endExpr);
		~BlockExpr();

		std::wstring Stringify() override;

		std::vector<struct Stmt *> stmts;
		Expr *endExpr;
	};

	struct AnonyObjExpr : public Expr
	{
		AnonyObjExpr();
		AnonyObjExpr(const std::vector<std::pair<std::wstring, Expr *>> &elements);
		~AnonyObjExpr();

		std::wstring Stringify() override;

		std::vector<std::pair<std::wstring, Expr *>> elements;
	};

	struct VarArgExpr : public Expr
	{
		VarArgExpr();
		VarArgExpr(IdentifierExpr* argName);
		~VarArgExpr();

		std::wstring Stringify() override;
		IdentifierExpr* argName;
	};

	struct FactorialExpr:public Expr
	{
		FactorialExpr();
		FactorialExpr(Expr* expr);
		~FactorialExpr();

		std::wstring Stringify() override;

		Expr* expr;
	};

	struct AppregateExpr:public  Expr
	{
		AppregateExpr();
		AppregateExpr(const std::vector<Expr*>& exprs);
		~AppregateExpr();

		std::wstring Stringify() override;

		std::vector<Expr*> exprs;
	};

	struct Stmt : public AstNode
	{
		Stmt(AstType type) : AstNode(type) {}
		virtual ~Stmt() {}

		virtual std::wstring Stringify() = 0;
	};

	struct ExprStmt : public Stmt
	{
		ExprStmt();
		ExprStmt(Expr *expr);
		~ExprStmt();
		std::wstring Stringify() override;

		Expr *expr;
	};

	struct VarStmt : public Stmt
	{
		enum class Privilege
		{
			MUTABLE,
			IMMUTABLE,
		};

		VarStmt();
		VarStmt(Privilege privilege, const std::vector<std::pair<Expr *, Expr *>> &variables);
		~VarStmt();

		std::wstring Stringify() override;

		Privilege privilege;
		std::vector<std::pair<Expr *, Expr *>> variables;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt();
		ReturnStmt(Expr * expr);
		~ReturnStmt();

		std::wstring Stringify() override;

		Expr * expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt();
		IfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
		~IfStmt();

		std::wstring Stringify() override;

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

		std::vector<Stmt *> stmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt();
		WhileStmt(Expr *condition, ScopeStmt *body, ScopeStmt *increment = nullptr);
		~WhileStmt();

		std::wstring Stringify() override;

		Expr *condition;
		ScopeStmt *body;
		ScopeStmt *increment;
	};

	struct BreakStmt : public Stmt
	{
		BreakStmt();
		~BreakStmt();

		std::wstring Stringify() override;
	};

	struct ContinueStmt : public Stmt
	{
		ContinueStmt();
		~ContinueStmt();

		std::wstring Stringify() override;
	};

	struct EnumStmt : public Stmt
	{
		EnumStmt();
		EnumStmt(IdentifierExpr *enumName, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems);
		~EnumStmt();

		std::wstring Stringify() override;

		IdentifierExpr *enumName;
		std::unordered_map<IdentifierExpr *, Expr *> enumItems;
	};

	struct ModuleStmt : public Stmt
	{
		ModuleStmt();
		ModuleStmt(IdentifierExpr *modName, const std::vector<Stmt *> &modItems);
		~ModuleStmt();

		std::wstring Stringify() override;

		IdentifierExpr *modName;
		std::vector<Stmt *> modItems;
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

		FunctionType type;
		IdentifierExpr *name;
		std::vector<IdentifierExpr *> parameters;
		ScopeStmt *body;
	};

	struct ClassStmt : public Stmt
	{
		ClassStmt();
		ClassStmt(std::wstring name,
				  std::vector<VarStmt *> varStmts,
				  std::vector<FunctionStmt *> fnStmts,
				  std::vector<FunctionStmt *> constructors = {},
				  std::vector<IdentifierExpr *> parentClasses = {});
		~ClassStmt();

		std::wstring Stringify() override;

		std::wstring name;
		std::vector<IdentifierExpr *> parentClasses;
		std::vector<FunctionStmt *> constructors;
		std::vector<VarStmt *> varStmts;
		std::vector<FunctionStmt *> fnStmts;
	};

	struct AstStmts : public Stmt
	{
		AstStmts();
		AstStmts(std::vector<Stmt *> stmts);
		~AstStmts();

		std::wstring Stringify() override;

		std::vector<Stmt *> stmts;
	};
}