#include "Ast.h"

namespace lws
{
	//----------------------Expressions-----------------------------

	IntNumExpr::IntNumExpr()
		: Expr(AST_INT), value(0)
	{
	}
	IntNumExpr::IntNumExpr(int64_t value)
		: Expr(AST_INT), value(value)
	{
	}
	IntNumExpr::~IntNumExpr()
	{
	}

	std::wstring IntNumExpr::ToString()
	{
		return std::to_wstring(value);
	}

	RealNumExpr::RealNumExpr()
		: Expr(AST_REAL), value(0.0)
	{
	}
	RealNumExpr::RealNumExpr(double value)
		: Expr(AST_REAL), value(value)
	{
	}
	RealNumExpr::~RealNumExpr()
	{
	}
	std::wstring RealNumExpr::ToString()
	{
		return std::to_wstring(value);
	}

	StrExpr::StrExpr()
		: Expr(AST_STR)
	{
	}
	StrExpr::StrExpr(std::wstring_view str)
		: Expr(AST_STR), value(str)
	{
	}

	StrExpr::~StrExpr()
	{
	}

	std::wstring StrExpr::ToString()
	{
		return L"\"" + value + L"\"";
	}

	NullExpr::NullExpr()
		: Expr(AST_NULL)
	{
	}
	NullExpr::~NullExpr()
	{
	}

	std::wstring NullExpr::ToString()
	{
		return L"null";
	}

	BoolExpr::BoolExpr()
		: Expr(AST_BOOL), value(false)
	{
	}
	BoolExpr::BoolExpr(bool value)
		: Expr(AST_BOOL), value(value)
	{
	}
	BoolExpr::~BoolExpr()
	{
	}

	std::wstring BoolExpr::ToString()
	{
		return value ? L"true" : L"false";
	}

	IdentifierExpr::IdentifierExpr()
		: Expr(AST_IDENTIFIER)
	{
	}
	IdentifierExpr::IdentifierExpr(std::wstring_view literal)
		: Expr(AST_IDENTIFIER), literal(literal)
	{
	}
	IdentifierExpr::~IdentifierExpr()
	{
	}

	std::wstring IdentifierExpr::ToString()
	{
		return literal;
	}

	VarDescExpr::VarDescExpr()
		: Expr(AST_VAR_DESC)
	{
	}
	VarDescExpr::VarDescExpr(std::wstring_view typeDesc, Expr *name)
		: Expr(AST_VAR_DESC), name(name), typeDesc(typeDesc)
	{
	}
	VarDescExpr::~VarDescExpr()
	{
		delete name;
		name = nullptr;
	}

	std::wstring VarDescExpr::ToString()
	{
		return name->ToString() + L":" + typeDesc;
	}

	ArrayExpr::ArrayExpr()
		: Expr(AST_ARRAY)
	{
	}
	ArrayExpr::ArrayExpr(std::vector<Expr *> elements) : Expr(AST_ARRAY), elements(elements)
	{
	}
	ArrayExpr::~ArrayExpr()
	{
		std::vector<Expr *>().swap(elements);
	}

	std::wstring ArrayExpr::ToString()
	{
		std::wstring result = L"[";

		if (!elements.empty())
		{
			for (auto e : elements)
				result += e->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L"]";
		return result;
	}

	DictExpr::DictExpr()
		: Expr(AST_DICT)
	{
	}
	DictExpr::DictExpr(const std::vector<std::pair<Expr *, Expr *>> &elements)
		: Expr(AST_DICT), elements(elements)
	{
	}
	DictExpr::~DictExpr()
	{
		std::vector<std::pair<Expr *, Expr *>>().swap(elements);
	}

	std::wstring DictExpr::ToString()
	{
		std::wstring result = L"{";

		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key->ToString() + L":" + value->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L"}";
		return result;
	}

	GroupExpr::GroupExpr()
		: Expr(AST_GROUP), expr(nullptr)
	{
	}
	GroupExpr::GroupExpr(Expr *expr)
		: Expr(AST_GROUP), expr(expr)
	{
	}
	GroupExpr::~GroupExpr()
	{
	}
	std::wstring GroupExpr::ToString()
	{
		return L"(" + expr->ToString() + L")";
	}

	PrefixExpr::PrefixExpr()
		: Expr(AST_PREFIX), right(nullptr)
	{
	}
	PrefixExpr::PrefixExpr(std::wstring_view op, Expr *right)
		: Expr(AST_PREFIX), op(op), right(right)
	{
	}
	PrefixExpr::~PrefixExpr()
	{
		delete right;
		right = nullptr;
	}

	std::wstring PrefixExpr::ToString()
	{
		return op + right->ToString();
	}

	InfixExpr::InfixExpr()
		: Expr(AST_INFIX), left(nullptr), right(nullptr)
	{
	}
	InfixExpr::InfixExpr(std::wstring_view op, Expr *left, Expr *right)
		: Expr(AST_INFIX), op(op), left(left), right(right)
	{
	}
	InfixExpr::~InfixExpr()
	{
		delete left;
		left = nullptr;

		delete right;
		right = nullptr;
	}

	std::wstring InfixExpr::ToString()
	{
		return left->ToString() + op + right->ToString();
	}

	PostfixExpr::PostfixExpr()
		: Expr(AST_POSTFIX), left(nullptr)
	{
	}
	PostfixExpr::PostfixExpr(Expr *left, std::wstring_view op)
		: Expr(AST_POSTFIX), left(left), op(op)
	{
	}
	PostfixExpr::~PostfixExpr()
	{
		delete left;
		left = nullptr;
	}
	std::wstring PostfixExpr::ToString()
	{
		return left->ToString() + op;
	}

	ConditionExpr::ConditionExpr()
		: Expr(AST_CONDITION), condition(nullptr), trueBranch(nullptr), falseBranch(nullptr)
	{
	}
	ConditionExpr::ConditionExpr(Expr *condition, Expr *trueBranch, Expr *falseBranch)
		: Expr(AST_CONDITION), condition(condition), trueBranch(trueBranch), falseBranch(falseBranch)
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

	std::wstring ConditionExpr::ToString()
	{
		return condition->ToString() + L"?" + trueBranch->ToString() + L":" + falseBranch->ToString();
	}

	IndexExpr::IndexExpr()
		: Expr(AST_INDEX), ds(nullptr), index(nullptr)
	{
	}
	IndexExpr::IndexExpr(Expr *ds, Expr *index)
		: Expr(AST_INDEX), ds(ds), index(index)
	{
	}
	IndexExpr::~IndexExpr()
	{
		delete ds;
		ds = nullptr;
		delete index;
		index = nullptr;
	}
	std::wstring IndexExpr::ToString()
	{
		return ds->ToString() + L"[" + index->ToString() + L"]";
	}

	RefExpr::RefExpr()
		: Expr(AST_REF), refExpr(nullptr)
	{
	}
	RefExpr::RefExpr(Expr *refExpr)
		: Expr(AST_REF), refExpr(refExpr)
	{
	}
	RefExpr::~RefExpr()
	{
	}

	std::wstring RefExpr::ToString()
	{
		return L"&" + refExpr->ToString();
	}

	LambdaExpr::LambdaExpr()
		: Expr(AST_LAMBDA), body(nullptr)
	{
	}
	LambdaExpr::LambdaExpr(std::vector<VarDescExpr *> parameters, ScopeStmt *body)
		: Expr(AST_LAMBDA), parameters(parameters), body(body)
	{
	}
	LambdaExpr::~LambdaExpr()
	{
		std::vector<VarDescExpr *>().swap(parameters);

		delete body;
		body = nullptr;
	}

	std::wstring LambdaExpr::ToString()
	{
		std::wstring result = L"fn(";
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L")";
		result += body->ToString();
		return result;
	}

	CallExpr::CallExpr()
		: Expr(AST_CALL)
	{
	}
	CallExpr::CallExpr(Expr *callee, std::vector<Expr *> arguments)
		: Expr(AST_CALL), callee(callee), arguments(arguments)
	{
	}
	CallExpr::~CallExpr()
	{
	}
	std::wstring CallExpr::ToString()
	{
		std::wstring result = callee->ToString() + L"(";

		if (!arguments.empty())
		{
			for (const auto &arg : arguments)
				result += arg->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L")";
		return result;
	}

	DotExpr::DotExpr()
		: Expr(AST_DOT), callee(nullptr), callMember(nullptr)
	{
	}
	DotExpr::DotExpr(Expr *callee, IdentifierExpr *callMember)
		: Expr(AST_DOT), callee(callee), callMember(callMember)
	{
	}
	DotExpr::~DotExpr()
	{
	}

	std::wstring DotExpr::ToString()
	{
		return callee->ToString() + L"." + callMember->ToString();
	}

	NewExpr::NewExpr()
		: Expr(AST_NEW)
	{
	}
	NewExpr::NewExpr(Expr *callee)
		: Expr(AST_NEW), callee(callee)
	{
	}
	NewExpr::~NewExpr()
	{
	}

	std::wstring NewExpr::ToString()
	{
		return L"new " + callee->ToString();
	}

	ThisExpr::ThisExpr()
		: Expr(AST_THIS)
	{
	}

	ThisExpr::~ThisExpr()
	{
	}

	std::wstring ThisExpr::ToString()
	{
		return L"this";
	}

	BaseExpr::BaseExpr(IdentifierExpr *callMember)
		: Expr(AST_BASE), callMember(callMember)
	{
	}
	BaseExpr::~BaseExpr()
	{
	}

	std::wstring BaseExpr::ToString()
	{
		return L"base." + callMember->ToString();
	}

	BlockExpr::BlockExpr()
		: Expr(AST_BLOCK)
	{
	}
	BlockExpr::BlockExpr(const std::vector<Stmt *> &stmts, Expr *endExpr)
		: Expr(AST_BLOCK), stmts(stmts), endExpr(endExpr)
	{
	}
	BlockExpr::~BlockExpr()
	{
	}

	std::wstring BlockExpr::ToString()
	{
		std::wstring result = L"({";
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		result += endExpr->ToString();
		result += L"})";
		return result;
	}

	AnonyObjExpr::AnonyObjExpr()
		: Expr(AST_ANONY_OBJ)
	{
	}
	AnonyObjExpr::AnonyObjExpr(const std::vector<std::pair<std::wstring, Expr *>> &elements)
		: Expr(AST_ANONY_OBJ), elements(elements)
	{
	}
	AnonyObjExpr::~AnonyObjExpr()
	{
		std::vector<std::pair<std::wstring, Expr *>>().swap(elements);
	}

	std::wstring AnonyObjExpr::ToString()
	{
		std::wstring result = L"{";
		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key + L":" + value->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L"}";
		return result;
	}
	VarArgExpr::VarArgExpr()
		: Expr(AST_VAR_ARG), argName(nullptr)
	{
	}
	VarArgExpr::VarArgExpr(IdentifierExpr *argName)
		: Expr(AST_VAR_ARG), argName(argName)
	{
	}
	VarArgExpr::~VarArgExpr()
	{
		delete argName;
	}

	std::wstring VarArgExpr::ToString()
	{
		return L"..." + (argName ? argName->ToString() : L"");
	}

	FactorialExpr::FactorialExpr()
		: Expr(AST_FACTORIAL), expr(nullptr)
	{
	}
	FactorialExpr::FactorialExpr(Expr *expr)
		: Expr(AST_FACTORIAL), expr(expr)
	{
	}
	FactorialExpr::~FactorialExpr()
	{
		delete expr;
		expr = nullptr;
	}

	std::wstring FactorialExpr::ToString()
	{
		return expr->ToString() + L"!";
	}

	AppregateExpr::AppregateExpr()
		: Expr(AST_APPREGATE)
	{
	}
	AppregateExpr::AppregateExpr(const std::vector<Expr *> &exprs)
		: Expr(AST_APPREGATE), exprs(exprs)
	{
	}
	AppregateExpr::~AppregateExpr()
	{
		std::vector<Expr *>().swap(exprs);
	}

	std::wstring AppregateExpr::ToString()
	{
		std::wstring result = L"(";
		if (!exprs.empty())
		{
			for (const auto &expr : exprs)
				result += expr->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}

		return result + L")";
	}

	//----------------------Statements-----------------------------

	ExprStmt::ExprStmt()
		: Stmt(AST_EXPR), expr(nullptr)
	{
	}
	ExprStmt::ExprStmt(Expr *expr)
		: Stmt(AST_EXPR), expr(expr)
	{
	}
	ExprStmt::~ExprStmt()
	{
		delete expr;
		expr = nullptr;
	}

	std::wstring ExprStmt::ToString()
	{
		return expr->ToString() + L";";
	}

	VarStmt::VarStmt()
		: Stmt(AST_VAR)
	{
	}
	VarStmt::VarStmt(Privilege privilege, const std::vector<std::pair<Expr *, Expr *>> &variables)
		: Stmt(AST_VAR), privilege(privilege), variables(variables)
	{
	}
	VarStmt::~VarStmt()
	{
		std::vector<std::pair<Expr *, Expr *>>().swap(variables);
	}

	std::wstring VarStmt::ToString()
	{
		std::wstring result;

		if (privilege == Privilege::IMMUTABLE)
			result += L"const ";
		else
			result += L"let ";

		if (!variables.empty())
		{
			for (auto [key, value] : variables)
				result += key->ToString() + L"=" + value->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		return result + L";";
	}

	ReturnStmt::ReturnStmt()
		: Stmt(AST_RETURN), expr(nullptr)
	{
	}
	ReturnStmt::ReturnStmt(Expr *expr)
		: Stmt(AST_RETURN), expr(expr)
	{
	}
	ReturnStmt::~ReturnStmt()
	{
		delete expr;
		expr = nullptr;
	}

	std::wstring ReturnStmt::ToString()
	{
		if (!expr)
			return L"return;";
		else
			return L"return " + expr->ToString() + L";";
	}

	IfStmt::IfStmt()
		: Stmt(AST_IF), condition(nullptr), thenBranch(nullptr), elseBranch(nullptr)
	{
	}
	IfStmt::IfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
		: Stmt(AST_IF),
		  condition(condition),
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

	std::wstring IfStmt::ToString()
	{
		std::wstring result;
		result = L"if(" + condition->ToString() + L")" + thenBranch->ToString();
		if (elseBranch != nullptr)
			result += L"else " + elseBranch->ToString();
		return result;
	}

	ScopeStmt::ScopeStmt()
		: Stmt(AST_SCOPE)
	{
	}
	ScopeStmt::ScopeStmt(std::vector<Stmt *> stmts)
		: Stmt(AST_SCOPE), stmts(stmts)
	{
	}
	ScopeStmt::~ScopeStmt()
	{
		std::vector<Stmt *>().swap(stmts);
	}

	std::wstring ScopeStmt::ToString()
	{
		std::wstring result = L"{";
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		result += L"}";
		return result;
	}

	WhileStmt::WhileStmt()
		: Stmt(AST_WHILE), condition(nullptr), body(nullptr), increment(nullptr)
	{
	}
	WhileStmt::WhileStmt(Expr *condition, ScopeStmt *body, ScopeStmt *increment)
		: Stmt(AST_WHILE), condition(condition), body(body), increment(increment)
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

	std::wstring WhileStmt::ToString()
	{
		std::wstring result = L"while(" + condition->ToString() + L"){" + body->ToString();
		if (increment)
			result += increment->ToString();
		return result += L"}";
	}

	BreakStmt::BreakStmt()
		: Stmt(AST_BREAK)
	{
	}
	BreakStmt::~BreakStmt()
	{
	}

	std::wstring BreakStmt::ToString()
	{
		return L"break;";
	}

	ContinueStmt::ContinueStmt()
		: Stmt(AST_CONTINUE)
	{
	}
	ContinueStmt::~ContinueStmt()
	{
	}

	std::wstring ContinueStmt::ToString()
	{
		return L"continue;";
	}

	EnumStmt::EnumStmt()
		: Stmt(AST_ENUM)
	{
	}
	EnumStmt::EnumStmt(IdentifierExpr *enumName, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems)
		: Stmt(AST_ENUM), enumName(enumName), enumItems(enumItems)
	{
	}
	EnumStmt::~EnumStmt()
	{
	}
	std::wstring EnumStmt::ToString()
	{
		std::wstring result = L"enum " + enumName->ToString() + L"{";

		if (!enumItems.empty())
		{
			for (auto [key, value] : enumItems)
				result += key->ToString() + L"=" + value->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		return result + L"}";
	}

	ModuleStmt::ModuleStmt()
		: Stmt(AST_MODULE)
	{
	}
	ModuleStmt::ModuleStmt(IdentifierExpr *modName, const std::vector<Stmt *> &modItems)
		: Stmt(AST_MODULE), modName(modName), modItems(modItems)
	{
	}
	ModuleStmt::~ModuleStmt()
	{
	}

	std::wstring ModuleStmt::ToString()
	{
		std::wstring result = L"module " + modName->ToString() + L"\n{\n";
		for (const auto &item : modItems)
			result += item->ToString() + L"\n";
		return result + L"}\n";
	}

	FunctionStmt::FunctionStmt()
		: Stmt(AST_FUNCTION), name(nullptr), body(nullptr)
	{
	}

	FunctionStmt::FunctionStmt(FunctionType type, IdentifierExpr *name, std::vector<VarDescExpr *> parameters, ScopeStmt *body)
		: Stmt(AST_FUNCTION), type(type), name(name), parameters(parameters), body(body)
	{
	}

	FunctionStmt::~FunctionStmt()
	{
		std::vector<VarDescExpr *>().swap(parameters);

		delete body;
		body = nullptr;
	}

	std::wstring FunctionStmt::ToString()
	{
		std::wstring result = L"fn " + name->ToString() + L"(";
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L")";
		result += body->ToString();
		return result;
	}

	ClassStmt::ClassStmt()
		: Stmt(AST_CLASS)
	{
	}
	ClassStmt::ClassStmt(std::wstring name,
						 std::vector<VarStmt *> varStmts,
						 std::vector<FunctionStmt *> fnStmts,
						 std::vector<FunctionStmt *> constructors,
						 std::vector<IdentifierExpr *> parentClasses)
		: Stmt(AST_CLASS),
		  name(name),
		  varStmts(varStmts),
		  fnStmts(fnStmts),
		  constructors(constructors),
		  parentClasses(parentClasses)
	{
	}
	ClassStmt::~ClassStmt()
	{
		std::vector<IdentifierExpr *>().swap(parentClasses);
		std::vector<FunctionStmt *>().swap(constructors);
		std::vector<VarStmt *>().swap(varStmts);
		std::vector<FunctionStmt *>().swap(fnStmts);
	}

	std::wstring ClassStmt::ToString()
	{
		std::wstring result = L"class " + name;
		if (!parentClasses.empty())
		{
			result += L":";
			for (const auto &parentClass : parentClasses)
				result += parentClass->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L"{";
		for (auto variableStmt : varStmts)
			result += variableStmt->ToString();
		for (auto fnStmt : fnStmts)
			result += fnStmt->ToString();
		return result + L"}";
	}

	AstStmts::AstStmts()
		: Stmt(AST_ASTSTMTS)
	{
	}
	AstStmts::AstStmts(std::vector<Stmt *> stmts)
		: Stmt(AST_ASTSTMTS), stmts(stmts)
	{
	}

	AstStmts::~AstStmts()
	{
		std::vector<Stmt *>().swap(stmts);
	}

	std::wstring AstStmts::ToString()
	{
		std::wstring result;
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		return result;
	}
}
