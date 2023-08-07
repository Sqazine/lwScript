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

	std::wstring IntNumExpr::Stringify()
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
	std::wstring RealNumExpr::Stringify()
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

	std::wstring StrExpr::Stringify()
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

	std::wstring NullExpr::Stringify()
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

	std::wstring BoolExpr::Stringify()
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

	std::wstring IdentifierExpr::Stringify()
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

	std::wstring VarDescExpr::Stringify()
	{
		return name->Stringify() + L":" + typeDesc;
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

	DictExpr::DictExpr()
		: Expr(AST_TABLE)
	{
	}
	DictExpr::DictExpr(const std::vector<std::pair<Expr *, Expr *>> &elements)
		: Expr(AST_TABLE), elements(elements)
	{
	}
	DictExpr::~DictExpr()
	{
		std::vector<std::pair<Expr *, Expr *>>().swap(elements);
	}

	std::wstring DictExpr::Stringify()
	{
		std::wstring result = L"{";

		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key->Stringify() + L":" + value->Stringify() + L",";
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
	std::wstring GroupExpr::Stringify()
	{
		return L"(" + expr->Stringify() + L")";
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

	std::wstring PrefixExpr::Stringify()
	{
		return op + right->Stringify();
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

	std::wstring InfixExpr::Stringify()
	{
		return left->Stringify() + op + right->Stringify();
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
	std::wstring PostfixExpr::Stringify()
	{
		return left->Stringify() + op;
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

	std::wstring ConditionExpr::Stringify()
	{
		return condition->Stringify() + L"?" + trueBranch->Stringify() + L":" + falseBranch->Stringify();
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
	std::wstring IndexExpr::Stringify()
	{
		return ds->Stringify() + L"[" + index->Stringify() + L"]";
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

	std::wstring RefExpr::Stringify()
	{
		return L"&" + refExpr->Stringify();
	}

	LambdaExpr::LambdaExpr()
		: Expr(AST_LAMBDA), body(nullptr)
	{
	}
	LambdaExpr::LambdaExpr(std::vector<IdentifierExpr *> parameters, ScopeStmt *body)
		: Expr(AST_LAMBDA), parameters(parameters), body(body)
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
	std::wstring CallExpr::Stringify()
	{
		std::wstring result = callee->Stringify() + L"(";

		if (!arguments.empty())
		{
			for (const auto &arg : arguments)
				result += arg->Stringify() + L",";
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

	std::wstring DotExpr::Stringify()
	{
		return callee->Stringify() + L"." + callMember->Stringify();
	}

	NewExpr::NewExpr()
		: Expr(AST_NEW)
	{
	}
	NewExpr::NewExpr(CallExpr *callee)
		: Expr(AST_NEW), callee(callee)
	{
	}
	NewExpr::~NewExpr()
	{
	}

	std::wstring NewExpr::Stringify()
	{
		return L"new " + callee->Stringify();
	}

	ThisExpr::ThisExpr()
		: Expr(AST_THIS)
	{
	}

	ThisExpr::~ThisExpr()
	{
	}

	std::wstring ThisExpr::Stringify()
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

	std::wstring BaseExpr::Stringify()
	{
		return L"base." + callMember->Stringify();
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

	std::wstring BlockExpr::Stringify()
	{
		std::wstring result = L"({";
		for (const auto &stmt : stmts)
			result += stmt->Stringify();
		result += endExpr->Stringify();
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

	std::wstring AnonyObjExpr::Stringify()
	{
		std::wstring result = L"{";
		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key + L":" + value->Stringify() + L",";
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

	std::wstring VarArgExpr::Stringify()
	{
		return L"..." + (argName ? argName->Stringify() : L"");
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

	std::wstring FactorialExpr::Stringify()
	{
		return expr->Stringify() + L"!";
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

	std::wstring ExprStmt::Stringify()
	{
		return expr->Stringify() + L";";
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

	std::wstring VarStmt::Stringify()
	{
		std::wstring result;

		if (privilege == Privilege::IMMUTABLE)
			result += L"const ";
		else
			result += L"let ";

		if (!variables.empty())
		{
			for (auto [key, value] : variables)
				result += key->Stringify() + L"=" + value->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		return result + L";";
	}

	ReturnStmt::ReturnStmt()
		: Stmt(AST_RETURN)
	{
	}
	ReturnStmt::ReturnStmt(const std::vector<Expr *> &exprs)
		: Stmt(AST_RETURN), exprs(exprs)
	{
	}
	ReturnStmt::~ReturnStmt()
	{
		std::vector<Expr *>().swap(exprs);
	}

	std::wstring ReturnStmt::Stringify()
	{
		if (exprs.empty())
			return L"return;";
		else
		{
			std::wstring result = L"return ";
			for (const auto &expr : exprs)
				result += expr->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
			return result + L";";
		}
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

	std::wstring IfStmt::Stringify()
	{
		std::wstring result;
		result = L"if(" + condition->Stringify() + L")" + thenBranch->Stringify();
		if (elseBranch != nullptr)
			result += L"else " + elseBranch->Stringify();
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

	std::wstring ScopeStmt::Stringify()
	{
		std::wstring result = L"{";
		for (const auto &stmt : stmts)
			result += stmt->Stringify();
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

	std::wstring WhileStmt::Stringify()
	{
		std::wstring result = L"while(" + condition->Stringify() + L"){" + body->Stringify();
		if (increment)
			result += increment->Stringify();
		return result += L"}";
	}

	BreakStmt::BreakStmt()
		: Stmt(AST_BREAK)
	{
	}
	BreakStmt::~BreakStmt()
	{
	}

	std::wstring BreakStmt::Stringify()
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

	std::wstring ContinueStmt::Stringify()
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

	std::wstring ModuleStmt::Stringify()
	{
		std::wstring result = L"module " + modName->Stringify() + L"\n{\n";
		for (const auto &item : modItems)
			result += item->Stringify() + L"\n";
		return result + L"}\n";
	}

	FunctionStmt::FunctionStmt()
		: Stmt(AST_FUNCTION), name(nullptr), body(nullptr)
	{
	}
	FunctionStmt::FunctionStmt(FunctionType type, IdentifierExpr *name, std::vector<IdentifierExpr *> parameters, ScopeStmt *body)
		: Stmt(AST_FUNCTION), type(type), name(name), parameters(parameters), body(body)
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

	std::wstring ClassStmt::Stringify()
	{
		std::wstring result = L"class " + name;
		if (!parentClasses.empty())
		{
			result += L":";
			for (const auto &parentClass : parentClasses)
				result += parentClass->Stringify() + L",";
			result = result.substr(0, result.size() - 1);
		}
		result += L"{";
		for (auto variableStmt : varStmts)
			result += variableStmt->Stringify();
		for (auto fnStmt : fnStmts)
			result += fnStmt->Stringify();
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

	std::wstring AstStmts::Stringify()
	{
		std::wstring result;
		for (const auto &stmt : stmts)
			result += stmt->Stringify();
		return result;
	}
}
