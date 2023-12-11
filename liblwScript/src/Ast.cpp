#include "Ast.h"

namespace lwscript
{
	//----------------------Expressions-----------------------------

	IntNumExpr::IntNumExpr(Token* tagToken)
		: Expr(tagToken, AST_INT), value(0)
	{
	}
	IntNumExpr::IntNumExpr(Token* tagToken, int64_t value)
		: Expr(tagToken, AST_INT), value(value)
	{
	}
	IntNumExpr::~IntNumExpr()
	{
	}

	std::wstring IntNumExpr::ToString()
	{
		return std::to_wstring(value);
	}

	RealNumExpr::RealNumExpr(Token* tagToken)
		: Expr(tagToken, AST_REAL), value(0.0)
	{
	}
	RealNumExpr::RealNumExpr(Token* tagToken, double value)
		: Expr(tagToken, AST_REAL), value(value)
	{
	}
	RealNumExpr::~RealNumExpr()
	{
	}
	std::wstring RealNumExpr::ToString()
	{
		return std::to_wstring(value);
	}

	StrExpr::StrExpr(Token* tagToken)
		: Expr(tagToken, AST_STR)
	{
	}
	StrExpr::StrExpr(Token* tagToken, std::wstring_view str)
		: Expr(tagToken, AST_STR), value(str)
	{
	}

	StrExpr::~StrExpr()
	{
	}

	std::wstring StrExpr::ToString()
	{
		return L"\"" + value + L"\"";
	}

	NullExpr::NullExpr(Token* tagToken)
		: Expr(tagToken, AST_NULL)
	{
	}
	NullExpr::~NullExpr()
	{
	}

	std::wstring NullExpr::ToString()
	{
		return L"null";
	}

	BoolExpr::BoolExpr(Token* tagToken)
		: Expr(tagToken, AST_BOOL), value(false)
	{
	}
	BoolExpr::BoolExpr(Token* tagToken, bool value)
		: Expr(tagToken, AST_BOOL), value(value)
	{
	}
	BoolExpr::~BoolExpr()
	{
	}

	std::wstring BoolExpr::ToString()
	{
		return value ? L"true" : L"false";
	}

	IdentifierExpr::IdentifierExpr(Token* tagToken)
		: Expr(tagToken, AST_IDENTIFIER)
	{
	}
	IdentifierExpr::IdentifierExpr(Token* tagToken, std::wstring_view literal)
		: Expr(tagToken, AST_IDENTIFIER), literal(literal)
	{
	}
	IdentifierExpr::~IdentifierExpr()
	{
	}

	std::wstring IdentifierExpr::ToString()
	{
		return literal;
	}

	VarDescExpr::VarDescExpr(Token* tagToken)
		: Expr(tagToken, AST_VAR_DESC), name(nullptr)
	{
	}
	VarDescExpr::VarDescExpr(Token* tagToken, std::wstring_view typeDesc, Expr *name)
		: Expr(tagToken, AST_VAR_DESC), name(name), typeDesc(typeDesc)
	{
	}
	VarDescExpr::~VarDescExpr()
	{
		SAFE_DELETE(name);
	}

	std::wstring VarDescExpr::ToString()
	{
		return name->ToString() + L":" + typeDesc;
	}

	ArrayExpr::ArrayExpr(Token* tagToken)
		: Expr(tagToken, AST_ARRAY)
	{
	}
	ArrayExpr::ArrayExpr(Token* tagToken, const std::vector<Expr *> &elements) : Expr(tagToken, AST_ARRAY), elements(elements)
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

	DictExpr::DictExpr(Token* tagToken)
		: Expr(tagToken, AST_DICT)
	{
	}
	DictExpr::DictExpr(Token* tagToken, const std::vector<std::pair<Expr *, Expr *>> &elements)
		: Expr(tagToken, AST_DICT), elements(elements)
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

	GroupExpr::GroupExpr(Token* tagToken)
		: Expr(tagToken, AST_GROUP), expr(nullptr)
	{
	}
	GroupExpr::GroupExpr(Token* tagToken, Expr *expr)
		: Expr(tagToken, AST_GROUP), expr(expr)
	{
	}
	GroupExpr::~GroupExpr()
	{
	}
	std::wstring GroupExpr::ToString()
	{
		return L"(" + expr->ToString() + L")";
	}

	PrefixExpr::PrefixExpr(Token* tagToken)
		: Expr(tagToken, AST_PREFIX), right(nullptr)
	{
	}
	PrefixExpr::PrefixExpr(Token* tagToken, std::wstring_view op, Expr *right)
		: Expr(tagToken, AST_PREFIX), op(op), right(right)
	{
	}
	PrefixExpr::~PrefixExpr()
	{
		SAFE_DELETE(right);
	}

	std::wstring PrefixExpr::ToString()
	{
		return op + right->ToString();
	}

	InfixExpr::InfixExpr(Token* tagToken)
		: Expr(tagToken, AST_INFIX), left(nullptr), right(nullptr)
	{
	}
	InfixExpr::InfixExpr(Token* tagToken, std::wstring_view op, Expr *left, Expr *right)
		: Expr(tagToken, AST_INFIX), op(op), left(left), right(right)
	{
	}
	InfixExpr::~InfixExpr()
	{
		SAFE_DELETE(left);
		SAFE_DELETE(right);
	}

	std::wstring InfixExpr::ToString()
	{
		return left->ToString() + op + right->ToString();
	}

	PostfixExpr::PostfixExpr(Token* tagToken)
		: Expr(tagToken, AST_POSTFIX), left(nullptr)
	{
	}
	PostfixExpr::PostfixExpr(Token* tagToken, Expr *left, std::wstring_view op)
		: Expr(tagToken, AST_POSTFIX), left(left), op(op)
	{
	}
	PostfixExpr::~PostfixExpr()
	{
		SAFE_DELETE(left);
	}
	std::wstring PostfixExpr::ToString()
	{
		return left->ToString() + op;
	}

	ConditionExpr::ConditionExpr(Token* tagToken)
		: Expr(tagToken, AST_CONDITION), condition(nullptr), trueBranch(nullptr), falseBranch(nullptr)
	{
	}
	ConditionExpr::ConditionExpr(Token* tagToken, Expr *condition, Expr *trueBranch, Expr *falseBranch)
		: Expr(tagToken, AST_CONDITION), condition(condition), trueBranch(trueBranch), falseBranch(falseBranch)
	{
	}
	ConditionExpr::~ConditionExpr()
	{
		SAFE_DELETE(condition);
		SAFE_DELETE(trueBranch);
		SAFE_DELETE(falseBranch);
	}

	std::wstring ConditionExpr::ToString()
	{
		return condition->ToString() + L"?" + trueBranch->ToString() + L":" + falseBranch->ToString();
	}

	IndexExpr::IndexExpr(Token* tagToken)
		: Expr(tagToken, AST_INDEX), ds(nullptr), index(nullptr)
	{
	}
	IndexExpr::IndexExpr(Token* tagToken, Expr *ds, Expr *index)
		: Expr(tagToken, AST_INDEX), ds(ds), index(index)
	{
	}
	IndexExpr::~IndexExpr()
	{
		SAFE_DELETE(ds);
		SAFE_DELETE(index);
	}
	std::wstring IndexExpr::ToString()
	{
		return ds->ToString() + L"[" + index->ToString() + L"]";
	}

	RefExpr::RefExpr(Token* tagToken)
		: Expr(tagToken, AST_REF), refExpr(nullptr)
	{
	}
	RefExpr::RefExpr(Token* tagToken, Expr *refExpr)
		: Expr(tagToken, AST_REF), refExpr(refExpr)
	{
	}
	RefExpr::~RefExpr()
	{
	}

	std::wstring RefExpr::ToString()
	{
		return L"&" + refExpr->ToString();
	}

	LambdaExpr::LambdaExpr(Token* tagToken)
		: Expr(tagToken, AST_LAMBDA), body(nullptr)
	{
	}
	LambdaExpr::LambdaExpr(Token* tagToken, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body)
		: Expr(tagToken, AST_LAMBDA), parameters(parameters), body(body)
	{
	}
	LambdaExpr::~LambdaExpr()
	{
		std::vector<VarDescExpr *>().swap(parameters);
		SAFE_DELETE(body);
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

	CallExpr::CallExpr(Token* tagToken)
		: Expr(tagToken, AST_CALL), callee(nullptr)
	{
	}
	CallExpr::CallExpr(Token* tagToken, Expr *callee, const std::vector<Expr *> &arguments)
		: Expr(tagToken, AST_CALL), callee(callee), arguments(arguments)
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

	DotExpr::DotExpr(Token* tagToken)
		: Expr(tagToken, AST_DOT), callee(nullptr), callMember(nullptr)
	{
	}
	DotExpr::DotExpr(Token* tagToken, Expr *callee, IdentifierExpr *callMember)
		: Expr(tagToken, AST_DOT), callee(callee), callMember(callMember)
	{
	}
	DotExpr::~DotExpr()
	{
	}

	std::wstring DotExpr::ToString()
	{
		return callee->ToString() + L"." + callMember->ToString();
	}

	NewExpr::NewExpr(Token* tagToken)
		: Expr(tagToken, AST_NEW), callee(nullptr)
	{
	}
	NewExpr::NewExpr(Token* tagToken, Expr *callee)
		: Expr(tagToken, AST_NEW), callee(callee)
	{
	}
	NewExpr::~NewExpr()
	{
	}

	std::wstring NewExpr::ToString()
	{
		return L"new " + callee->ToString();
	}

	ThisExpr::ThisExpr(Token* tagToken)
		: Expr(tagToken, AST_THIS)
	{
	}

	ThisExpr::~ThisExpr()
	{
	}

	std::wstring ThisExpr::ToString()
	{
		return L"this";
	}

	BaseExpr::BaseExpr(Token* tagToken, IdentifierExpr *callMember)
		: Expr(tagToken, AST_BASE), callMember(callMember)
	{
	}
	BaseExpr::~BaseExpr()
	{
	}

	std::wstring BaseExpr::ToString()
	{
		return L"base." + callMember->ToString();
	}

	BlockExpr::BlockExpr(Token* tagToken)
		: Expr(tagToken, AST_BLOCK), endExpr(nullptr)
	{
	}
	BlockExpr::BlockExpr(Token* tagToken, const std::vector<Stmt *> &stmts, Expr *endExpr)
		: Expr(tagToken, AST_BLOCK), stmts(stmts), endExpr(endExpr)
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

	AnonyObjExpr::AnonyObjExpr(Token* tagToken)
		: Expr(tagToken, AST_ANONY_OBJ)
	{
	}
	AnonyObjExpr::AnonyObjExpr(Token* tagToken, const std::vector<std::pair<std::wstring, Expr *>> &elements)
		: Expr(tagToken, AST_ANONY_OBJ), elements(elements)
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
	VarArgExpr::VarArgExpr(Token* tagToken)
		: Expr(tagToken, AST_VAR_ARG), argName(nullptr)
	{
	}
	VarArgExpr::VarArgExpr(Token* tagToken, IdentifierExpr *argName)
		: Expr(tagToken, AST_VAR_ARG), argName(argName)
	{
	}
	VarArgExpr::~VarArgExpr()
	{
		SAFE_DELETE(argName);
	}

	std::wstring VarArgExpr::ToString()
	{
		return L"..." + (argName ? argName->ToString() : L"");
	}

	FactorialExpr::FactorialExpr(Token* tagToken)
		: Expr(tagToken, AST_FACTORIAL), expr(nullptr)
	{
	}
	FactorialExpr::FactorialExpr(Token* tagToken, Expr *expr)
		: Expr(tagToken, AST_FACTORIAL), expr(expr)
	{
	}
	FactorialExpr::~FactorialExpr()
	{
		SAFE_DELETE(expr);
	}

	std::wstring FactorialExpr::ToString()
	{
		return expr->ToString() + L"!";
	}

	AppregateExpr::AppregateExpr(Token* tagToken)
		: Expr(tagToken, AST_APPREGATE)
	{
	}
	AppregateExpr::AppregateExpr(Token* tagToken, const std::vector<Expr *> &exprs)
		: Expr(tagToken, AST_APPREGATE), exprs(exprs)
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

	ExprStmt::ExprStmt(Token* tagToken)
		: Stmt(tagToken, AST_EXPR), expr(nullptr)
	{
	}
	ExprStmt::ExprStmt(Token* tagToken, Expr *expr)
		: Stmt(tagToken, AST_EXPR), expr(expr)
	{
	}
	ExprStmt::~ExprStmt()
	{
		SAFE_DELETE(expr);
	}

	std::wstring ExprStmt::ToString()
	{
		return expr->ToString() + L";";
	}

	VarStmt::VarStmt(Token* tagToken)
		: Stmt(tagToken, AST_VAR), privilege(Privilege::MUTABLE)
	{
	}
	VarStmt::VarStmt(Token* tagToken, Privilege privilege, const std::vector<std::pair<Expr *, Expr *>> &variables)
		: Stmt(tagToken, AST_VAR), privilege(privilege), variables(variables)
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

	ReturnStmt::ReturnStmt(Token* tagToken)
		: Stmt(tagToken, AST_RETURN), expr(nullptr)
	{
	}
	ReturnStmt::ReturnStmt(Token* tagToken, Expr *expr)
		: Stmt(tagToken, AST_RETURN), expr(expr)
	{
	}
	ReturnStmt::~ReturnStmt()
	{
		SAFE_DELETE(expr);
	}

	std::wstring ReturnStmt::ToString()
	{
		if (!expr)
			return L"return;";
		else
			return L"return " + expr->ToString() + L";";
	}

	IfStmt::IfStmt(Token* tagToken)
		: Stmt(tagToken, AST_IF), condition(nullptr), thenBranch(nullptr), elseBranch(nullptr)
	{
	}
	IfStmt::IfStmt(Token* tagToken, Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
		: Stmt(tagToken, AST_IF),
		  condition(condition),
		  thenBranch(thenBranch),
		  elseBranch(elseBranch)
	{
	}
	IfStmt::~IfStmt()
	{
		SAFE_DELETE(condition);
		SAFE_DELETE(thenBranch);
		SAFE_DELETE(elseBranch);
	}

	std::wstring IfStmt::ToString()
	{
		std::wstring result;
		result = L"if(" + condition->ToString() + L")" + thenBranch->ToString();
		if (elseBranch != nullptr)
			result += L"else " + elseBranch->ToString();
		return result;
	}

	ScopeStmt::ScopeStmt(Token* tagToken)
		: Stmt(tagToken, AST_SCOPE)
	{
	}
	ScopeStmt::ScopeStmt(Token* tagToken, const std::vector<Stmt *> &stmts)
		: Stmt(tagToken, AST_SCOPE), stmts(stmts)
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

	WhileStmt::WhileStmt(Token* tagToken)
		: Stmt(tagToken, AST_WHILE), condition(nullptr), body(nullptr), increment(nullptr)
	{
	}
	WhileStmt::WhileStmt(Token* tagToken, Expr *condition, ScopeStmt *body, ScopeStmt *increment)
		: Stmt(tagToken, AST_WHILE), condition(condition), body(body), increment(increment)
	{
	}
	WhileStmt::~WhileStmt()
	{
		SAFE_DELETE(condition);
		SAFE_DELETE(body);
		SAFE_DELETE(increment);
	}

	std::wstring WhileStmt::ToString()
	{
		std::wstring result = L"while(" + condition->ToString() + L"){" + body->ToString();
		if (increment)
			result += increment->ToString();
		return result += L"}";
	}

	BreakStmt::BreakStmt(Token* tagToken)
		: Stmt(tagToken, AST_BREAK)
	{
	}
	BreakStmt::~BreakStmt()
	{
	}

	std::wstring BreakStmt::ToString()
	{
		return L"break;";
	}

	ContinueStmt::ContinueStmt(Token* tagToken)
		: Stmt(tagToken, AST_CONTINUE)
	{
	}
	ContinueStmt::~ContinueStmt()
	{
	}

	std::wstring ContinueStmt::ToString()
	{
		return L"continue;";
	}

	EnumStmt::EnumStmt(Token* tagToken)
		: Stmt(tagToken, AST_ENUM), name(nullptr)
	{
	}
	EnumStmt::EnumStmt(Token* tagToken, IdentifierExpr *name, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems)
		: Stmt(tagToken, AST_ENUM), name(name), enumItems(enumItems)
	{
	}
	EnumStmt::~EnumStmt()
	{
	}
	std::wstring EnumStmt::ToString()
	{
		std::wstring result = L"enum " + name->ToString() + L"{";

		if (!enumItems.empty())
		{
			for (auto [key, value] : enumItems)
				result += key->ToString() + L"=" + value->ToString() + L",";
			result = result.substr(0, result.size() - 1);
		}
		return result + L"}";
	}

	ModuleStmt::ModuleStmt(Token* tagToken)
		: Stmt(tagToken, AST_MODULE), name(nullptr)
	{
	}
	ModuleStmt::ModuleStmt(Token* tagToken,
						   IdentifierExpr *name,
						   const std::vector<VarStmt *> &varItems,
						   const std::vector<ClassStmt *> &classItems,
						   const std::vector<ModuleStmt *> &moduleItems,
						   const std::vector<EnumStmt *> &enumItems,
						   const std::vector<FunctionStmt *> &functionItems)
		: Stmt(tagToken, AST_MODULE), name(name),
		  varItems(varItems),
		  classItems(classItems),
		  moduleItems(moduleItems),
		  enumItems(enumItems),
		  functionItems(functionItems)
	{
	}
	ModuleStmt::~ModuleStmt()
	{
	}

	std::wstring ModuleStmt::ToString()
	{
		std::wstring result = L"module " + name->ToString() + L"\n{\n";
		for (const auto &item : varItems)
			result += item->ToString() + L"\n";
		for (const auto &item : classItems)
			result += item->ToString() + L"\n";
		for (const auto &item : moduleItems)
			result += item->ToString() + L"\n";
		for (const auto &item : enumItems)
			result += item->ToString() + L"\n";
		for (const auto &item : functionItems)
			result += item->ToString() + L"\n";
		return result + L"}\n";
	}

	FunctionStmt::FunctionStmt(Token* tagToken)
		: Stmt(tagToken, AST_FUNCTION), name(nullptr), body(nullptr), type(FunctionType::FUNCTION)
	{
	}

	FunctionStmt::FunctionStmt(Token* tagToken, FunctionType type, IdentifierExpr *name, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body)
		: Stmt(tagToken, AST_FUNCTION), type(type), name(name), parameters(parameters), body(body)
	{
	}

	FunctionStmt::~FunctionStmt()
	{
		std::vector<VarDescExpr *>().swap(parameters);
		SAFE_DELETE(body);
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

	ClassStmt::ClassStmt(Token* tagToken)
		: Stmt(tagToken, AST_CLASS)
	{
	}
	ClassStmt::ClassStmt(Token* tagToken,
						 std::wstring name,
				  const std::vector<VarStmt *> &varItems,
				  const std::vector<FunctionStmt *> &fnItems,
				  const std::vector<EnumStmt *> &enumItems,
				  const std::vector<FunctionStmt *> &constructors,
				  const std::vector<IdentifierExpr *> &parentClasses)
		: Stmt(tagToken, AST_CLASS),
		  name(name),
		  varItems(varItems),
		  fnItems(fnItems),
		  enumItems(enumItems),
		  constructors(constructors),
		  parentClasses(parentClasses)
	{
	}
	ClassStmt::~ClassStmt()
	{
		std::vector<IdentifierExpr *>().swap(parentClasses);
		std::vector<FunctionStmt *>().swap(constructors);
		std::vector<VarStmt *>().swap(varItems);
		std::vector<FunctionStmt *>().swap(fnItems);
		std::vector<EnumStmt *>().swap(enumItems);
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
		for (auto enumStmt : enumItems)
			result += enumStmt->ToString();
		for (auto variableStmt : varItems)
			result += variableStmt->ToString();
		for (auto fnStmt : fnItems)
			result += fnStmt->ToString();
		return result + L"}";
	}

	AstStmts::AstStmts(Token* tagToken)
		: Stmt(tagToken, AST_ASTSTMTS)
	{
	}
	AstStmts::AstStmts(Token* tagToken, std::vector<Stmt *> stmts)
		: Stmt(tagToken, AST_ASTSTMTS), stmts(stmts)
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
