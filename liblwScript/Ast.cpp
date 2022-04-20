#include "Ast.h"

namespace lws
{
	//----------------------Expressions-----------------------------

	IntNumExpr::IntNumExpr()
		: value(0)
	{
	}
	IntNumExpr::IntNumExpr(int64_t value)
		: value(value)
	{
	}
	IntNumExpr::~IntNumExpr()
	{
	}

	std::wstring IntNumExpr::Stringify()
	{
		return std::to_wstring(value);
	}
	AstType IntNumExpr::Type()
	{
		return AST_INT;
	}

	RealNumExpr::RealNumExpr()
		: value(0.0)
	{
	}
	RealNumExpr::RealNumExpr(double value)
		: value(value)
	{
	}
	RealNumExpr::~RealNumExpr()
	{
	}
	std::wstring RealNumExpr::Stringify()
	{
		return std::to_wstring(value);
	}
	AstType RealNumExpr::Type()
	{
		return AST_REAL;
	}

	StrExpr::StrExpr()
	{
	}
	StrExpr::StrExpr(std::wstring_view str)
		: value(str)
	{
	}

	StrExpr::~StrExpr()
	{
	}

	std::wstring StrExpr::Stringify()
	{
		return L"\"" + value + L"\"";
	}

	AstType StrExpr::Type()
	{
		return AST_STR;
	}

	NullExpr::NullExpr()
	{
	}
	NullExpr::~NullExpr()
	{
	}

	std::wstring NullExpr::Stringify()
	{
		return L"null";
	}
	AstType NullExpr::Type()
	{
		return AST_NULL;
	}

	BoolExpr::BoolExpr()
		: value(false)
	{
	}
	BoolExpr::BoolExpr(bool value)
		: value(value)
	{
	}
	BoolExpr::~BoolExpr()
	{
	}

	std::wstring BoolExpr::Stringify()
	{
		return value ? L"true" : L"false";
	}
	AstType BoolExpr::Type()
	{
		return AST_BOOL;
	}

	IdentifierExpr::IdentifierExpr()
	{
	}
	IdentifierExpr::IdentifierExpr(std::wstring_view literal)
		: literal(literal)
	{
	}
	IdentifierExpr::~IdentifierExpr()
	{
	}

	std::wstring IdentifierExpr::Stringify()
	{
		return literal;
	}
	AstType IdentifierExpr::Type()
	{
		return AST_IDENTIFIER;
	}

	ArrayExpr::ArrayExpr()
	{
	}
	ArrayExpr::ArrayExpr(std::vector<Expr *> elements) : elements(elements)
	{
	}
	ArrayExpr::~ArrayExpr()
	{
		std::vector<Expr *>().swap(elements);
	}

	std::wstring ArrayExpr::Stringify()
	{
		std::wstring result = L"[";

		if (!elements.empty())
		{
			for (auto e : elements)
				result += e->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L"]";
		return result;
	}
	AstType ArrayExpr::Type()
	{
		return AST_ARRAY;
	}

	TableExpr::TableExpr()
	{
	}
	TableExpr::TableExpr(std::unordered_map<Expr *, Expr *> elements)
		: elements(elements)
	{
	}
	TableExpr::~TableExpr()
	{
		std::unordered_map<Expr *, Expr *>().swap(elements);
	}

	std::wstring TableExpr::Stringify()
	{
		std::wstring result = L"{";

		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key->Stringify() + L":" + value->Stringify();
			result = result.substr(0, result.size() - 1);
		}
		result += L"}";
		return result;
	}
	AstType TableExpr::Type()
	{
		return AST_TABLE;
	}

	GroupExpr::GroupExpr()
		: expr(nullptr)
	{
	}
	GroupExpr::GroupExpr(Expr *expr)
		: expr(expr)
	{
	}
	GroupExpr::~GroupExpr()
	{
	}
	std::wstring GroupExpr::Stringify()
	{
		return L"(" + expr->Stringify() + L")";
	}
	AstType GroupExpr::Type()
	{
		return AST_GROUP;
	}

	PrefixExpr::PrefixExpr()
		: right(nullptr)
	{
	}
	PrefixExpr::PrefixExpr(std::wstring_view op, Expr *right)
		: op(op), right(right)
	{
	}
	PrefixExpr::~PrefixExpr()
	{
		delete right;
		right = nullptr;
	}

	std::wstring PrefixExpr::Stringify()
	{
		return op + right->Stringify();
	}
	AstType PrefixExpr::Type()
	{
		return AST_PREFIX;
	}


	InfixExpr::InfixExpr()
		: left(nullptr), right(nullptr)
	{
	}
	InfixExpr::InfixExpr(std::wstring_view op, Expr *left, Expr *right)
		: op(op), left(left), right(right)
	{
	}
	InfixExpr::~InfixExpr()
	{
		delete left;
		left = nullptr;

		delete right;
		right = nullptr;
	}

	std::wstring InfixExpr::Stringify()
	{
		return left->Stringify() + op + right->Stringify();
	}
	AstType InfixExpr::Type()
	{
		return AST_INFIX;
	}

	PostfixExpr::PostfixExpr()
		: left(nullptr)
	{
	}
	PostfixExpr::PostfixExpr(Expr *left, std::wstring_view op)
		: left(left), op(op)
	{
	}
	PostfixExpr::~PostfixExpr()
	{
		delete left;
		left = nullptr;
	}
	std::wstring PostfixExpr::Stringify()
	{
		return left->Stringify() + op;
	}
	AstType PostfixExpr::Type()
	{
		return AST_POSTFIX;
	}

	ConditionExpr::ConditionExpr()
		: condition(nullptr), trueBranch(nullptr), falseBranch(nullptr)
	{
	}
	ConditionExpr::ConditionExpr(Expr *condition, Expr *trueBranch, Expr *falseBranch)
		: condition(condition), trueBranch(trueBranch), falseBranch(falseBranch)
	{
	}
	ConditionExpr::~ConditionExpr()
	{
		delete condition;
		condition = nullptr;

		delete trueBranch;
		trueBranch = nullptr;

		delete falseBranch;
		trueBranch = nullptr;
	}

	std::wstring ConditionExpr::Stringify()
	{
		return condition->Stringify() + L"?" + trueBranch->Stringify() + L":" + falseBranch->Stringify();
	}
	AstType ConditionExpr::Type()
	{
		return AST_CONDITION;
	}

	IndexExpr::IndexExpr()
		: ds(nullptr), index(nullptr)
	{
	}
	IndexExpr::IndexExpr(Expr *ds, Expr *index)
		: ds(ds), index(index)
	{
	}
	IndexExpr::~IndexExpr()
	{
		delete ds;
		ds = nullptr;
		delete index;
		index = nullptr;
	}
	std::wstring IndexExpr::Stringify()
	{
		return ds->Stringify() + L"[" + index->Stringify() + L"]";
	}

	AstType IndexExpr::Type()
	{
		return AST_INDEX;
	}

	RefExpr::RefExpr()
		: refExpr(nullptr)
	{
	}
	RefExpr::RefExpr(Expr *refExpr)
		: refExpr(refExpr) {}
	RefExpr::~RefExpr()
	{
	}

	std::wstring RefExpr::Stringify()
	{
		return L"&" + refExpr->Stringify();
	}

	AstType RefExpr::Type()
	{
		return AST_REF;
	}

	LambdaExpr::LambdaExpr()
		: body(nullptr)
	{
	}
	LambdaExpr::LambdaExpr(std::vector<IdentifierExpr *> parameters, ScopeStmt *body)
		: parameters(parameters), body(body)
	{
	}
	LambdaExpr::~LambdaExpr()
	{
		std::vector<IdentifierExpr *>().swap(parameters);

		delete body;
		body = nullptr;
	}

	std::wstring LambdaExpr::Stringify()
	{
		std::wstring result = L"fn(";
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L")";
		result += body->Stringify();
		return result;
	}
	AstType LambdaExpr::Type()
	{
		return AST_LAMBDA;
	}

	FunctionCallExpr::FunctionCallExpr()
	{
	}
	FunctionCallExpr::FunctionCallExpr(Expr *name, std::vector<Expr *> arguments)
		: name(name), arguments(arguments)
	{
	}
	FunctionCallExpr::~FunctionCallExpr()
	{
	}
	std::wstring FunctionCallExpr::Stringify()
	{
		std::wstring result = name->Stringify() + L"(";

		if (!arguments.empty())
		{
			for (const auto &arg : arguments)
				result += arg->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L")";
		return result;
	}
	AstType FunctionCallExpr::Type()
	{
		return AST_FUNCTION_CALL;
	}

	FieldCallExpr::FieldCallExpr()
		: callee(nullptr), callMember(nullptr)
	{
	}
	FieldCallExpr::FieldCallExpr(Expr *callee, Expr *callMember)
		: callee(callee), callMember(callMember)
	{
	}
	FieldCallExpr::~FieldCallExpr()
	{
	}

	std::wstring FieldCallExpr::Stringify()
	{
		return callee->Stringify() + L"." + callMember->Stringify();
	}
	AstType FieldCallExpr::Type()
	{
		return AST_FIELD_CALL;
	}

	//----------------------Statements-----------------------------

	ExprStmt::ExprStmt()
		: expr(nullptr)
	{
	}
	ExprStmt::ExprStmt(Expr *expr)
		: expr(expr)
	{
	}
	ExprStmt::~ExprStmt()
	{
		delete expr;
		expr = nullptr;
	}

	std::wstring ExprStmt::Stringify()
	{
		return expr->Stringify() + L";";
	}
	AstType ExprStmt::Type()
	{
		return AST_EXPR;
	}

	LetStmt::LetStmt()
	{
	}
	LetStmt::LetStmt(const std::unordered_map<IdentifierExpr *, Expr *> &variables)
		: variables(variables)
	{
	}
	LetStmt::~LetStmt()
	{
		std::unordered_map<IdentifierExpr *, Expr *>().swap(variables);
	}

	std::wstring LetStmt::Stringify()
	{
		std::wstring result = L"let ";
		if (!variables.empty())
		{
			for (auto [key, value] : variables)
				result += key->Stringify() + L"=" + value->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		return result + L";";
	}

	AstType LetStmt::Type()
	{
		return AST_LET;
	}

	ConstStmt::ConstStmt()
	{
	}
	ConstStmt::ConstStmt(const std::unordered_map<IdentifierExpr *, Expr *> &consts)
		: consts(consts)
	{
	}
	ConstStmt::~ConstStmt()
	{
		std::unordered_map<IdentifierExpr *, Expr *>().swap(consts);
	}

	std::wstring ConstStmt::Stringify()
	{
		std::wstring result = L"const ";
		if (!consts.empty())
		{
			for (auto [key, value] : consts)
				result += key->Stringify() + L"=" + value->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		return result + L";";
	}

	AstType ConstStmt::Type()
	{
		return AST_CONST;
	}

	ReturnStmt::ReturnStmt()
		: expr(nullptr)
	{
	}
	ReturnStmt::ReturnStmt(Expr *expr)
		: expr(expr)
	{
	}
	ReturnStmt::~ReturnStmt()
	{
		delete expr;
		expr = nullptr;
	}

	std::wstring ReturnStmt::Stringify()
	{
		if (expr)
			return L"return " + expr->Stringify() + L";";
		else
			return L"return;";
	}
	AstType ReturnStmt::Type()
	{
		return AST_RETURN;
	}

	IfStmt::IfStmt()
		: condition(nullptr), thenBranch(nullptr), elseBranch(nullptr)
	{
	}
	IfStmt::IfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
		: condition(condition),
		  thenBranch(thenBranch),
		  elseBranch(elseBranch)
	{
	}
	IfStmt::~IfStmt()
	{
		delete condition;
		condition = nullptr;
		delete thenBranch;
		thenBranch = nullptr;
		delete elseBranch;
		elseBranch = nullptr;
	}

	std::wstring IfStmt::Stringify()
	{
		std::wstring result;
		result = L"if(" + condition->Stringify() + L")" + thenBranch->Stringify();
		if (elseBranch != nullptr)
			result += L"else " + elseBranch->Stringify();
		return result;
	}
	AstType IfStmt::Type()
	{
		return AST_IF;
	}

	ScopeStmt::ScopeStmt()
	{
	}
	ScopeStmt::ScopeStmt(std::vector<Stmt *> stmts)
		: stmts(stmts) {}
	ScopeStmt::~ScopeStmt()
	{
		std::vector<Stmt *>().swap(stmts);
	}

	std::wstring ScopeStmt::Stringify()
	{
		std::wstring result = L"{";
		for (const auto &stmt : stmts)
			result += stmt->Stringify();
		result += L"}";
		return result;
	}

	AstType ScopeStmt::Type()
	{
		return AST_SCOPE;
	}

	WhileStmt::WhileStmt()
		: condition(nullptr), body(nullptr), increment(nullptr)
	{
	}
	WhileStmt::WhileStmt(Expr *condition, ScopeStmt *body, ScopeStmt *increment)
		: condition(condition), body(body), increment(increment)
	{
	}
	WhileStmt::~WhileStmt()
	{
		delete condition;
		condition = nullptr;
		delete body;
		body = nullptr;
		delete increment;
		increment = nullptr;
	}

	std::wstring WhileStmt::Stringify()
	{
		std::wstring result = L"while(" + condition->Stringify() + L"){" + body->Stringify();
		if (increment)
			result += increment->Stringify();
		return result += L"}";
	}
	AstType WhileStmt::Type()
	{
		return AST_WHILE;
	}

	BreakStmt::BreakStmt()
	{
	}
	BreakStmt::~BreakStmt()
	{
	}

	std::wstring BreakStmt::Stringify()
	{
		return L"break;";
	}
	AstType BreakStmt::Type()
	{
		return AST_BREAK;
	}


	ContinueStmt::ContinueStmt()
	{
	}
	ContinueStmt::~ContinueStmt()
	{
	}

	std::wstring ContinueStmt::Stringify()
	{
		return L"continue;";
	}
	AstType ContinueStmt::Type()
	{
		return AST_CONTINUE;
	}


	EnumStmt::EnumStmt()
	{
	}
	EnumStmt::EnumStmt(IdentifierExpr *enumName, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems)
		: enumName(enumName), enumItems(enumItems)
	{
	}
	EnumStmt::~EnumStmt()
	{
	}
	std::wstring EnumStmt::Stringify()
	{
		std::wstring result = L"enum " + enumName->Stringify() + L"{";

		if (!enumItems.empty())
		{
			for (auto [key, value] : enumItems)
				result += key->Stringify() + L"=" + value->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		return result + L"}";
	}
	AstType EnumStmt::Type()
	{
		return AST_ENUM;
	}

	FunctionStmt::FunctionStmt()
		: name(nullptr), body(nullptr)
	{
	}
	FunctionStmt::FunctionStmt(IdentifierExpr *name, std::vector<IdentifierExpr *> parameters, ScopeStmt *body)
		: name(name), parameters(parameters), body(body)
	{
	}
	FunctionStmt::~FunctionStmt()
	{
		std::vector<IdentifierExpr *>().swap(parameters);

		delete body;
		body = nullptr;
	}

	std::wstring FunctionStmt::Stringify()
	{
		std::wstring result = L"fn " + name->Stringify() + L"(";
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L")";
		result += body->Stringify();
		return result;
	}
	AstType FunctionStmt::Type()
	{
		return AST_FUNCTION;
	}

	FieldStmt::FieldStmt()
	{
	}
	FieldStmt::FieldStmt(std::wstring name,
						 std::vector<LetStmt *> letStmts,
						 std::vector<ConstStmt *> constStmts,
						 std::vector<FunctionStmt *> fnStmts,
						 std::vector<IdentifierExpr *> containedFields)
		: name(name),
		  letStmts(letStmts),
		  constStmts(constStmts),
		  fnStmts(fnStmts),
		  containedFields(containedFields)
	{
	}
	FieldStmt::~FieldStmt()
	{
		std::vector<IdentifierExpr *>().swap(containedFields);
		std::vector<LetStmt *>().swap(letStmts);
		std::vector<ConstStmt *>().swap(constStmts);
		std::vector<FunctionStmt *>().swap(fnStmts);
	}

	std::wstring FieldStmt::Stringify()
	{
		std::wstring result = L"field " + name;
		if (!containedFields.empty())
		{
			result += L":";
			for (const auto &containedField : containedFields)
				result += containedField->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L"{";
		for (auto constStmt : constStmts)
			result += constStmt->Stringify();
		for (auto letStmt : letStmts)
			result += letStmt->Stringify();
		for (auto fnStmt : fnStmts)
			result += fnStmt->Stringify();
		return result + L"}";
	}
	AstType FieldStmt::Type()
	{
		return AST_FIELD;
	}

	AstStmts::AstStmts()
	{
	}
	AstStmts::AstStmts(std::vector<Stmt *> stmts)
		: stmts(stmts) {}
	AstStmts::~AstStmts()
	{
		std::vector<Stmt *>().swap(stmts);
	}

	std::wstring AstStmts::Stringify()
	{
		std::wstring result;
		for (const auto &stmt : stmts)
			result += stmt->Stringify();
		return result;
	}
	AstType AstStmts::Type()
	{
		return AST_ASTSTMTS;
	}
}
