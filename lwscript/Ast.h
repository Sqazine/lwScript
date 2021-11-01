#pragma once
#include <string>

#include <unordered_map>
#include <vector>
#include <memory>
namespace lws
{
	enum class AstType
	{
		//expr
		INTEGER,
		FLOATING,
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
		FUNCTION_CALL,
		CLASS_CALL,
		//stmt
		LET,
		EXPR,
		RETURN,
		IF,
		SCOPE,
		WHILE,
		FUNCTION,
		CLASS,
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

	struct IntegerExpr : public Expr
	{
		IntegerExpr() : value(0) {}
		IntegerExpr(int64_t value) : value(value) {}
		~IntegerExpr() {}

		std::string Stringify() override { return std::to_string(value); }
		AstType Type() override { return AstType::INTEGER; }

		int64_t value;
	};

	struct FloatingExpr : public Expr
	{
		FloatingExpr() : value(0.0) {}
		FloatingExpr(double value) : value(value) {}
		~FloatingExpr() {}

		std::string Stringify() override { return std::to_string(value); }
		AstType Type() override { return AstType::FLOATING; }

		double value;
	};

	struct StrExpr : public Expr
	{
		StrExpr() {}
		StrExpr(std::string_view str) : value(str) {}

		std::string Stringify() override { return "\"" + value + "\""; }
		AstType Type() override { return AstType::STR; }

		std::string value;
	};

	struct NilExpr : public Expr
	{
		NilExpr() {}
		~NilExpr() {}

		std::string Stringify() override { return "nil"; }
		AstType Type() override { return AstType::NIL; }
	};

	struct BoolExpr : public Expr
	{
		BoolExpr() : value(false) {}
		BoolExpr(bool value) : value(value) {}
		~BoolExpr() {}

		std::string Stringify() override { return value ? "true" : "false"; }
		AstType Type() override { return AstType::BOOL; }
		bool value;
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr() {}
		IdentifierExpr(std::string_view literal) : literal(literal) {}
		~IdentifierExpr() {}

		std::string Stringify() override { return literal; }
		AstType Type() override { return AstType::IDENTIFIER; }

		std::string literal;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr() {}
		ArrayExpr(std::vector<Expr*> elements) : elements(elements) {}
		~ArrayExpr()
		{
			std::vector<Expr*>().swap(elements);
		}

		std::string Stringify() override
		{
			std::string result = "[";

			if (!elements.empty())
			{
				for (auto e : elements)
					result += e->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += "]";
			return result;
		}
		AstType Type() override { return AstType::ARRAY; }

		std::vector<Expr*> elements;
	};

	struct TableExpr : public Expr
	{
		TableExpr() {}
		TableExpr(std::unordered_map<Expr*, Expr*> elements) : elements(elements) {}
		~TableExpr()
		{
			std::unordered_map<Expr*, Expr*>().swap(elements);
		}

		std::string Stringify() override
		{
			std::string result = "{";

			if (!elements.empty())
			{
				for (auto [key, value] : elements)
					result += key->Stringify() + ":" + value->Stringify();
				result = result.substr(0, result.size() - 1);
			}
			result += "}";
			return result;
		}
		AstType Type() override { return AstType::TABLE; }

		std::unordered_map<Expr*, Expr*> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr() : expr(nullptr) {}
		GroupExpr(Expr* expr) : expr(expr) {}
		~GroupExpr() {}

		std::string Stringify() override { return "(" + expr->Stringify() + ")"; }
		AstType Type() override { return AstType::GROUP; }

		Expr* expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr() : right(nullptr) {}
		PrefixExpr(std::string_view op, Expr* right) : op(op), right(right) {}
		~PrefixExpr()
		{
			delete right;
			right = nullptr;
		}

		std::string Stringify() override { return op + right->Stringify(); }
		AstType Type() override { return AstType::PREFIX; }

		std::string op;
		Expr* right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr() : left(nullptr), right(nullptr) {}
		InfixExpr(std::string_view op, Expr* left, Expr* right) : op(op), left(left), right(right) {}
		~InfixExpr()
		{
			delete left;
			left = nullptr;

			delete right;
			right = nullptr;
		}

		std::string Stringify() override { return left->Stringify() + op + right->Stringify(); }
		AstType Type() override { return AstType::INFIX; }

		std::string op;
		Expr* left;
		Expr* right;
	};

	struct ConditionExpr : public Expr
	{
		ConditionExpr() : condition(nullptr), trueBranch(nullptr), falseBranch(nullptr) {}
		ConditionExpr(Expr* condition, Expr* left, Expr* right) : condition(condition), trueBranch(trueBranch), falseBranch(falseBranch) {}
		~ConditionExpr()
		{
			delete condition;
			condition = nullptr;

			delete trueBranch;
			trueBranch = nullptr;

			delete falseBranch;
			trueBranch = nullptr;
		}

		std::string Stringify() override { return condition->Stringify() + "?" + trueBranch->Stringify() + ":" + falseBranch->Stringify(); }
		AstType Type() override { return AstType::CONDITION; }

		Expr* condition;
		Expr* trueBranch;
		Expr* falseBranch;
	};

	struct IndexExpr : public Expr
	{
		IndexExpr() : array(nullptr), index(nullptr) {}
		IndexExpr(Expr* array, Expr* index) : array(array), index(index) {}
		~IndexExpr()
		{
			delete array;
			array = nullptr;
			delete index;
			index = nullptr;
		}
		std::string Stringify() override { return array->Stringify() + "[" + index->Stringify() + "]"; }

		AstType Type() override { return AstType::INDEX; }

		Expr* array;
		Expr* index;
	};

	struct RefExpr : public Expr
	{
		RefExpr() {}
		RefExpr(std::string_view refName) : refName(refName) {};
		~RefExpr() {}

		std::string Stringify() override { return "ref " + refName; }

		AstType Type() override { return AstType::REF; }

		std::string refName;
	};

	struct FunctionCallExpr : public Expr
	{
		FunctionCallExpr() {}
		FunctionCallExpr(std::string name, std::vector<Expr*> arguments) : name(name), arguments(arguments) {}
		~FunctionCallExpr() {}

		std::string Stringify() override
		{
			std::string result = name + "(";

			if (!arguments.empty())
			{
				for (const auto& arg : arguments)
					result += arg->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += ")";
			return result;
		}
		AstType Type() override { return AstType::FUNCTION_CALL; }

		std::string name;
		std::vector<Expr*> arguments;
	};

	struct StructCallExpr : public Expr
	{
		StructCallExpr() : callee(nullptr), callMember(nullptr) {}
		StructCallExpr(Expr* callee, Expr* callMember) : callee(callee), callMember(callMember) {}
		~StructCallExpr() {}

		std::string Stringify() override { return callee->Stringify() + "." + callMember->Stringify(); }
		AstType Type() override { return AstType::CLASS_CALL; }

		Expr* callee;
		Expr* callMember;
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
		ExprStmt() : expr(nullptr) {}
		ExprStmt(Expr* expr) : expr(expr) {}
		~ExprStmt()
		{
			delete expr;
			expr = nullptr;
		}

		std::string Stringify() override { return expr->Stringify() + ";"; }
		AstType Type() override { return AstType::EXPR; }

		Expr* expr;
	};

	struct LetStmt : public Stmt
	{
		LetStmt() {}
		LetStmt(const std::unordered_map<IdentifierExpr*, Expr*>& variables) : variables(variables) {}
		~LetStmt() { std::unordered_map<IdentifierExpr*, Expr*>().swap(variables); }

		std::string Stringify() override
		{
			std::string result = "let ";
			if (!variables.empty())
			{
				for (auto [key, value] : variables)
					result += key->Stringify() + "=" + value->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			return result + ";";
		}

		AstType Type() override { return AstType::LET; }

		std::unordered_map<IdentifierExpr*, Expr*> variables;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt() : expr(nullptr) {}
		ReturnStmt(Expr* expr) : expr(expr) {}
		~ReturnStmt()
		{
			delete expr;
			expr = nullptr;
		}

		std::string Stringify() override { return "return " + expr->Stringify() + ";"; }
		AstType Type() override { return AstType::RETURN; }

		Expr* expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt() : condition(nullptr), thenBranch(nullptr), elseBranch(nullptr) {}
		IfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch)
			: condition(condition),
			thenBranch(thenBranch),
			elseBranch(elseBranch)
		{
		}
		~IfStmt()
		{
			delete condition;
			condition = nullptr;
			delete thenBranch;
			thenBranch = nullptr;
			delete elseBranch;
			elseBranch = nullptr;
		}

		std::string Stringify() override
		{
			std::string result;
			result = "if(" + condition->Stringify() + ")" + thenBranch->Stringify();
			if (elseBranch != nullptr)
				result += "else " + elseBranch->Stringify();
			return result;
		}
		AstType Type() override { return AstType::IF; }

		Expr* condition;
		Stmt* thenBranch;
		Stmt* elseBranch;
	};

	struct ScopeStmt : public Stmt
	{
		ScopeStmt() {}
		ScopeStmt(std::vector<Stmt*> stmts) : stmts(stmts) {}
		~ScopeStmt() { std::vector<Stmt*>().swap(stmts); }

		std::string Stringify() override
		{
			std::string result = "{";
			for (const auto& stmt : stmts)
				result += stmt->Stringify();
			result += "}";
			return result;
		}

		AstType Type() override { return AstType::SCOPE; }
		std::vector<Stmt*> stmts;
	};

	struct FunctionExpr : public Expr
	{
		FunctionExpr() : body(nullptr) {}
		FunctionExpr(std::vector<IdentifierExpr*> parameters, ScopeStmt* body) : parameters(parameters), body(body) {}
		~FunctionExpr()
		{
			std::vector<IdentifierExpr*>().swap(parameters);

			delete body;
			body = nullptr;
		}

		std::string Stringify() override
		{
			std::string result = "function (";
			if (!parameters.empty())
			{
				for (auto param : parameters)
					result += param->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += ")";
			result += body->Stringify();
			return result;
		}
		AstType Type() override { return AstType::FUNCTION; }

		std::vector<IdentifierExpr*> parameters;
		ScopeStmt* body;
	};
	struct ClassStmt : public Stmt
	{
		ClassStmt() {}
		ClassStmt(std::string name, std::vector<LetStmt*> letStmts) : name(name), letStmts(letStmts) {}
		~ClassStmt() { std::vector<LetStmt*>().swap(letStmts); }

		std::string Stringify() override
		{
			std::string result = "struct " + name + "{\n";
			if (!letStmts.empty())
			{
				for (auto letStmt : letStmts)
					result += letStmt->Stringify() + "\n";
				result = result.substr(0, result.size() - 1);
			}
			return result + "\n}";
		}
		AstType Type() override { return AstType::CLASS; }

		std::string name;
		std::vector<LetStmt*> letStmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt() : condition(nullptr), body(nullptr) {}
		WhileStmt(Expr* condition, Stmt* body) : condition(condition), body(body) {}
		~WhileStmt()
		{
			delete condition;
			condition = nullptr;
			delete body;
			body = nullptr;
		}

		std::string Stringify() override
		{
			return "while(" + condition->Stringify() + ")" + body->Stringify();
		}
		AstType Type() override { return AstType::WHILE; }

		Expr* condition;
		Stmt* body;
	};

	struct AstStmts : public Stmt
	{
		AstStmts() {}
		AstStmts(std::vector<Stmt*> stmts) : stmts(stmts) {}
		~AstStmts() { std::vector<Stmt*>().swap(stmts); }

		std::string Stringify() override
		{
			std::string result;
			for (const auto& stmt : stmts)
				result += stmt->Stringify();
			return result;
		}
		AstType Type() override { return AstType::ASTSTMTS; }

		std::vector<Stmt*> stmts;
	};

	static NilExpr* nilExpr = new NilExpr();
	static BoolExpr* trueExpr = new BoolExpr(true);
	static BoolExpr* falseExpr = new BoolExpr(false);
}