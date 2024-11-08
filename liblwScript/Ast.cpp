#include "Ast.h"

namespace lwscript
{
	//----------------------Expressions-----------------------------

	LiteralExpr::LiteralExpr(Token *tagToken)
		: Expr(tagToken, AstKind::LITERAL), literalType(Type::NIL)
	{
	}
	LiteralExpr::LiteralExpr(Token *tagToken, int64_t value)
		: Expr(tagToken, AstKind::LITERAL), literalType(Type::INTEGER), iValue(value)
	{
	}
	LiteralExpr::LiteralExpr(Token *tagToken, double value)
		: Expr(tagToken, AstKind::LITERAL), literalType(Type::FLOATING), dValue(value)
	{
	}
	LiteralExpr::LiteralExpr(Token *tagToken, bool value)
		: Expr(tagToken, AstKind::LITERAL), literalType(Type::BOOLEAN), boolean(value)
	{
	}
	LiteralExpr::LiteralExpr(Token *tagToken, std::wstring_view value)
		: Expr(tagToken, AstKind::LITERAL), literalType(Type::STRING), str(value)
	{
	}
	LiteralExpr::~LiteralExpr()
	{
	}

	std::wstring LiteralExpr::ToString()
	{
		switch (literalType)
		{
		case Type::INTEGER:
			return std::to_wstring(iValue);
		case Type::FLOATING:
			return std::to_wstring(dValue);
		case Type::BOOLEAN:
			return boolean ? L"true" : L"false";
		case Type::CHARACTER:
			return std::wstring({character});
		case Type::STRING:
			return str;
		default:
			return L"null";
		}
	}

	IdentifierExpr::IdentifierExpr(Token *tagToken)
		: Expr(tagToken, AstKind::IDENTIFIER)
	{
	}
	IdentifierExpr::IdentifierExpr(Token *tagToken, std::wstring_view literal)
		: Expr(tagToken, AstKind::IDENTIFIER), literal(literal)
	{
	}
	IdentifierExpr::~IdentifierExpr()
	{
	}

	std::wstring IdentifierExpr::ToString()
	{
		return literal;
	}

	VarDescExpr::VarDescExpr(Token *tagToken)
		: Expr(tagToken, AstKind::VAR_DESC), name(nullptr)
	{
	}
	VarDescExpr::VarDescExpr(Token *tagToken, std::wstring_view typeDesc, Expr *name)
		: Expr(tagToken, AstKind::VAR_DESC), name(name), typeDesc(typeDesc)
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

	ArrayExpr::ArrayExpr(Token *tagToken)
		: Expr(tagToken, AstKind::ARRAY)
	{
	}
	ArrayExpr::ArrayExpr(Token *tagToken, const std::vector<Expr *> &elements) : Expr(tagToken, AstKind::ARRAY), elements(elements)
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

	DictExpr::DictExpr(Token *tagToken)
		: Expr(tagToken, AstKind::DICT)
	{
	}
	DictExpr::DictExpr(Token *tagToken, const std::vector<std::pair<Expr *, Expr *>> &elements)
		: Expr(tagToken, AstKind::DICT), elements(elements)
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

	GroupExpr::GroupExpr(Token *tagToken)
		: Expr(tagToken, AstKind::GROUP), expr(nullptr)
	{
	}
	GroupExpr::GroupExpr(Token *tagToken, Expr *expr)
		: Expr(tagToken, AstKind::GROUP), expr(expr)
	{
	}
	GroupExpr::~GroupExpr()
	{
		SAFE_DELETE(expr);
	}
	std::wstring GroupExpr::ToString()
	{
		return L"(" + expr->ToString() + L")";
	}

	PrefixExpr::PrefixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::PREFIX), right(nullptr)
	{
	}
	PrefixExpr::PrefixExpr(Token *tagToken, std::wstring_view op, Expr *right)
		: Expr(tagToken, AstKind::PREFIX), op(op), right(right)
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

	InfixExpr::InfixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::INFIX), left(nullptr), right(nullptr)
	{
	}
	InfixExpr::InfixExpr(Token *tagToken, std::wstring_view op, Expr *left, Expr *right)
		: Expr(tagToken, AstKind::INFIX), op(op), left(left), right(right)
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

	PostfixExpr::PostfixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::POSTFIX), left(nullptr)
	{
	}
	PostfixExpr::PostfixExpr(Token *tagToken, Expr *left, std::wstring_view op)
		: Expr(tagToken, AstKind::POSTFIX), left(left), op(op)
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

	ConditionExpr::ConditionExpr(Token *tagToken)
		: Expr(tagToken, AstKind::CONDITION), condition(nullptr), trueBranch(nullptr), falseBranch(nullptr)
	{
	}
	ConditionExpr::ConditionExpr(Token *tagToken, Expr *condition, Expr *trueBranch, Expr *falseBranch)
		: Expr(tagToken, AstKind::CONDITION), condition(condition), trueBranch(trueBranch), falseBranch(falseBranch)
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

	IndexExpr::IndexExpr(Token *tagToken)
		: Expr(tagToken, AstKind::INDEX), ds(nullptr), index(nullptr)
	{
	}
	IndexExpr::IndexExpr(Token *tagToken, Expr *ds, Expr *index)
		: Expr(tagToken, AstKind::INDEX), ds(ds), index(index)
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

	RefExpr::RefExpr(Token *tagToken)
		: Expr(tagToken, AstKind::REF), refExpr(nullptr)
	{
	}
	RefExpr::RefExpr(Token *tagToken, Expr *refExpr)
		: Expr(tagToken, AstKind::REF), refExpr(refExpr)
	{
	}
	RefExpr::~RefExpr()
	{
		SAFE_DELETE(refExpr);
	}

	std::wstring RefExpr::ToString()
	{
		return L"&" + refExpr->ToString();
	}

	LambdaExpr::LambdaExpr(Token *tagToken)
		: Expr(tagToken, AstKind::LAMBDA), body(nullptr)
	{
	}
	LambdaExpr::LambdaExpr(Token *tagToken, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body)
		: Expr(tagToken, AstKind::LAMBDA), parameters(parameters), body(body)
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

	CallExpr::CallExpr(Token *tagToken)
		: Expr(tagToken, AstKind::CALL), callee(nullptr)
	{
	}
	CallExpr::CallExpr(Token *tagToken, Expr *callee, const std::vector<Expr *> &arguments)
		: Expr(tagToken, AstKind::CALL), callee(callee), arguments(arguments)
	{
	}
	CallExpr::~CallExpr()
	{
		SAFE_DELETE(callee);
		std::vector<Expr *>().swap(arguments);
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

	DotExpr::DotExpr(Token *tagToken)
		: Expr(tagToken, AstKind::DOT), callee(nullptr), callMember(nullptr)
	{
	}
	DotExpr::DotExpr(Token *tagToken, Expr *callee, IdentifierExpr *callMember)
		: Expr(tagToken, AstKind::DOT), callee(callee), callMember(callMember)
	{
	}
	DotExpr::~DotExpr()
	{
		SAFE_DELETE(callee);
		SAFE_DELETE(callMember);
	}

	std::wstring DotExpr::ToString()
	{
		return callee->ToString() + L"." + callMember->ToString();
	}

	NewExpr::NewExpr(Token *tagToken)
		: Expr(tagToken, AstKind::NEW), callee(nullptr)
	{
	}
	NewExpr::NewExpr(Token *tagToken, Expr *callee)
		: Expr(tagToken, AstKind::NEW), callee(callee)
	{
	}
	NewExpr::~NewExpr()
	{
		SAFE_DELETE(callee);
	}

	std::wstring NewExpr::ToString()
	{
		return L"new " + callee->ToString();
	}

	ThisExpr::ThisExpr(Token *tagToken)
		: Expr(tagToken, AstKind::THIS)
	{
	}

	ThisExpr::~ThisExpr()
	{
	}

	std::wstring ThisExpr::ToString()
	{
		return L"this";
	}

	BaseExpr::BaseExpr(Token *tagToken, IdentifierExpr *callMember)
		: Expr(tagToken, AstKind::BASE), callMember(callMember)
	{
	}
	BaseExpr::~BaseExpr()
	{
		SAFE_DELETE(callMember);
	}

	std::wstring BaseExpr::ToString()
	{
		return L"base." + callMember->ToString();
	}

	CompoundExpr::CompoundExpr(Token *tagToken)
		: Expr(tagToken, AstKind::COMPOUND), endExpr(nullptr)
	{
	}
	CompoundExpr::CompoundExpr(Token *tagToken, const std::vector<Stmt *> &stmts, Expr *endExpr)
		: Expr(tagToken, AstKind::COMPOUND), stmts(stmts), endExpr(endExpr)
	{
	}
	CompoundExpr::~CompoundExpr()
	{
		SAFE_DELETE(endExpr);
		std::vector<Stmt *>().swap(stmts);
	}

	std::wstring CompoundExpr::ToString()
	{
		std::wstring result = L"({";
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		result += endExpr->ToString();
		result += L"})";
		return result;
	}

	StructExpr::StructExpr(Token *tagToken)
		: Expr(tagToken, AstKind::STRUCT)
	{
	}
	StructExpr::StructExpr(Token *tagToken, const std::vector<std::pair<std::wstring, Expr *>> &elements)
		: Expr(tagToken, AstKind::STRUCT), elements(elements)
	{
	}
	StructExpr::~StructExpr()
	{
		std::vector<std::pair<std::wstring, Expr *>>().swap(elements);
	}

	std::wstring StructExpr::ToString()
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
	VarArgExpr::VarArgExpr(Token *tagToken)
		: Expr(tagToken, AstKind::VAR_ARG), argName(nullptr)
	{
	}
	VarArgExpr::VarArgExpr(Token *tagToken, IdentifierExpr *argName)
		: Expr(tagToken, AstKind::VAR_ARG), argName(argName)
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

	FactorialExpr::FactorialExpr(Token *tagToken)
		: Expr(tagToken, AstKind::FACTORIAL), expr(nullptr)
	{
	}
	FactorialExpr::FactorialExpr(Token *tagToken, Expr *expr)
		: Expr(tagToken, AstKind::FACTORIAL), expr(expr)
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

	AppregateExpr::AppregateExpr(Token *tagToken)
		: Expr(tagToken, AstKind::APPREGATE)
	{
	}
	AppregateExpr::AppregateExpr(Token *tagToken, const std::vector<Expr *> &exprs)
		: Expr(tagToken, AstKind::APPREGATE), exprs(exprs)
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

	ExprStmt::ExprStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::EXPR), expr(nullptr)
	{
	}
	ExprStmt::ExprStmt(Token *tagToken, Expr *expr)
		: Stmt(tagToken, AstKind::EXPR), expr(expr)
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

	VarStmt::VarStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::VAR), privilege(Privilege::MUTABLE)
	{
	}
	VarStmt::VarStmt(Token *tagToken, Privilege privilege, const std::vector<std::pair<Expr *, Expr *>> &variables)
		: Stmt(tagToken, AstKind::VAR), privilege(privilege), variables(variables)
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

	ReturnStmt::ReturnStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::RETURN), expr(nullptr)
	{
	}
	ReturnStmt::ReturnStmt(Token *tagToken, Expr *expr)
		: Stmt(tagToken, AstKind::RETURN), expr(expr)
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

	IfStmt::IfStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::IF), condition(nullptr), thenBranch(nullptr), elseBranch(nullptr)
	{
	}
	IfStmt::IfStmt(Token *tagToken, Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
		: Stmt(tagToken, AstKind::IF),
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

	ScopeStmt::ScopeStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::SCOPE)
	{
	}
	ScopeStmt::ScopeStmt(Token *tagToken, const std::vector<Stmt *> &stmts)
		: Stmt(tagToken, AstKind::SCOPE), stmts(stmts)
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

	WhileStmt::WhileStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::WHILE), condition(nullptr), body(nullptr), increment(nullptr)
	{
	}
	WhileStmt::WhileStmt(Token *tagToken, Expr *condition, ScopeStmt *body, ScopeStmt *increment)
		: Stmt(tagToken, AstKind::WHILE), condition(condition), body(body), increment(increment)
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

	BreakStmt::BreakStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::BREAK)
	{
	}
	BreakStmt::~BreakStmt()
	{
	}

	std::wstring BreakStmt::ToString()
	{
		return L"break;";
	}

	ContinueStmt::ContinueStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::CONTINUE)
	{
	}
	ContinueStmt::~ContinueStmt()
	{
	}

	std::wstring ContinueStmt::ToString()
	{
		return L"continue;";
	}

	EnumStmt::EnumStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::ENUM), name(nullptr)
	{
	}
	EnumStmt::EnumStmt(Token *tagToken, IdentifierExpr *name, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems)
		: Stmt(tagToken, AstKind::ENUM), name(name), enumItems(enumItems)
	{
	}
	EnumStmt::~EnumStmt()
	{
		SAFE_DELETE(name);
		std::unordered_map<IdentifierExpr *, Expr *>().swap(enumItems);
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

	ModuleStmt::ModuleStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::MODULE), name(nullptr)
	{
	}
	ModuleStmt::ModuleStmt(Token *tagToken,
						   IdentifierExpr *name,
						   const std::vector<VarStmt *> &varItems,
						   const std::vector<ClassStmt *> &classItems,
						   const std::vector<ModuleStmt *> &moduleItems,
						   const std::vector<EnumStmt *> &enumItems,
						   const std::vector<FunctionStmt *> &functionItems)
		: Stmt(tagToken, AstKind::MODULE), name(name),
		  varItems(varItems),
		  classItems(classItems),
		  moduleItems(moduleItems),
		  enumItems(enumItems),
		  functionItems(functionItems)
	{
	}
	ModuleStmt::~ModuleStmt()
	{
		SAFE_DELETE(name);
		std::vector<VarStmt *>().swap(varItems);
		std::vector<ClassStmt *>().swap(classItems);
		std::vector<ModuleStmt *>().swap(moduleItems);
		std::vector<EnumStmt *>().swap(enumItems);
		std::vector<FunctionStmt *>().swap(functionItems);
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

	FunctionStmt::FunctionStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::FUNCTION), name(nullptr), body(nullptr), functionKind(FunctionKind::FUNCTION)
	{
	}

	FunctionStmt::FunctionStmt(Token *tagToken, FunctionKind kind, IdentifierExpr *name, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body)
		: Stmt(tagToken, AstKind::FUNCTION), functionKind(kind), name(name), parameters(parameters), body(body)
	{
	}

	FunctionStmt::~FunctionStmt()
	{
		SAFE_DELETE(name);
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

	ClassStmt::ClassStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::CLASS)
	{
	}
	ClassStmt::ClassStmt(Token *tagToken,
						 std::wstring name,
						 const std::vector<VarStmt *> &varItems,
						 const std::vector<FunctionStmt *> &fnItems,
						 const std::vector<EnumStmt *> &enumItems,
						 const std::vector<FunctionStmt *> &constructors,
						 const std::vector<IdentifierExpr *> &parentClasses)
		: Stmt(tagToken, AstKind::CLASS),
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

	AstStmts::AstStmts(Token *tagToken)
		: Stmt(tagToken, AstKind::ASTSTMTS)
	{
	}
	AstStmts::AstStmts(Token *tagToken, std::vector<Stmt *> stmts)
		: Stmt(tagToken, AstKind::ASTSTMTS), stmts(stmts)
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
