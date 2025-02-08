#include "Ast.h"

namespace lwscript
{
	//----------------------Expressions-----------------------------

	LiteralExpr::LiteralExpr(Token *tagToken)
		: Expr(tagToken, AstKind::LITERAL)
	{
	}
	LiteralExpr::LiteralExpr(Token *tagToken, int64_t value)
		: Expr(tagToken, AstKind::LITERAL), i64Value(value)
	{
		type = Type(TEXT("i64"), tagToken->sourceLocation);
	}
	LiteralExpr::LiteralExpr(Token *tagToken, double value)
		: Expr(tagToken, AstKind::LITERAL), f64Value(value)
	{
		type = Type(TEXT("f64"), tagToken->sourceLocation);
	}
	LiteralExpr::LiteralExpr(Token *tagToken, bool value)
		: Expr(tagToken, AstKind::LITERAL), boolean(value)
	{
		type = Type(TEXT("bool"), tagToken->sourceLocation);
	}
	LiteralExpr::LiteralExpr(Token *tagToken, STD_STRING_VIEW value)
		: Expr(tagToken, AstKind::LITERAL), str(value)
	{
		type = Type(TEXT("string"), tagToken->sourceLocation);
	}
	LiteralExpr::~LiteralExpr()
	{
	}

	STD_STRING LiteralExpr::ToString()
	{
		switch (type.GetKind())
		{
		case TypeKind::I8:
		case TypeKind::I16:
		case TypeKind::I32:
		case TypeKind::I64:
		case TypeKind::U8:
		case TypeKind::U16:
		case TypeKind::U32:
		case TypeKind::U64:
			return TO_STRING(i64Value);
		case TypeKind::F32:
		case TypeKind::F64:
			return TO_STRING(f64Value);
		case TypeKind::BOOL:
			return boolean ? TEXT("true") : TEXT("false");
		case TypeKind::CHAR:
			return STD_STRING({character});
		case TypeKind::STRING:
			return str;
		default:
			return TEXT("null");
		}
	}

	IdentifierExpr::IdentifierExpr(Token *tagToken)
		: Expr(tagToken, AstKind::IDENTIFIER)
	{
	}
	IdentifierExpr::IdentifierExpr(Token *tagToken, STD_STRING_VIEW literal)
		: Expr(tagToken, AstKind::IDENTIFIER), literal(literal)
	{
	}
	IdentifierExpr::~IdentifierExpr()
	{
	}

	STD_STRING IdentifierExpr::ToString()
	{
		return literal;
	}

	VarDescExpr::VarDescExpr(Token *tagToken)
		: Expr(tagToken, AstKind::VAR_DESC), name(nullptr)
	{
		type = Type(Type(TEXT("any")));
	}
	VarDescExpr::VarDescExpr(Token *tagToken, const Type &type, Expr *name)
		: Expr(tagToken, AstKind::VAR_DESC), name(name)
	{
		this->type = type;
	}
	VarDescExpr::~VarDescExpr()
	{
		SAFE_DELETE(name);
	}

	STD_STRING VarDescExpr::ToString()
	{
		return name->ToString() + TEXT(":") + STD_STRING(type.GetName().data());
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

	STD_STRING ArrayExpr::ToString()
	{
		STD_STRING result = TEXT("[");

		if (!elements.empty())
		{
			for (auto e : elements)
				result += e->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("]");
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

	STD_STRING DictExpr::ToString()
	{
		STD_STRING result = TEXT("{");

		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key->ToString() + TEXT(":") + value->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("}");
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
	STD_STRING GroupExpr::ToString()
	{
		return TEXT("(") + expr->ToString() + TEXT(")");
	}

	PrefixExpr::PrefixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::PREFIX), right(nullptr)
	{
	}
	PrefixExpr::PrefixExpr(Token *tagToken, STD_STRING_VIEW op, Expr *right)
		: Expr(tagToken, AstKind::PREFIX), op(op), right(right)
	{
	}
	PrefixExpr::~PrefixExpr()
	{
		SAFE_DELETE(right);
	}

	STD_STRING PrefixExpr::ToString()
	{
		return op + right->ToString();
	}

	InfixExpr::InfixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::INFIX), left(nullptr), right(nullptr)
	{
	}
	InfixExpr::InfixExpr(Token *tagToken, STD_STRING_VIEW op, Expr *left, Expr *right)
		: Expr(tagToken, AstKind::INFIX), op(op), left(left), right(right)
	{
	}
	InfixExpr::~InfixExpr()
	{
		SAFE_DELETE(left);
		SAFE_DELETE(right);
	}

	STD_STRING InfixExpr::ToString()
	{
		return left->ToString() + op + right->ToString();
	}

	PostfixExpr::PostfixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::POSTFIX), left(nullptr)
	{
	}
	PostfixExpr::PostfixExpr(Token *tagToken, Expr *left, STD_STRING_VIEW op)
		: Expr(tagToken, AstKind::POSTFIX), left(left), op(op)
	{
	}
	PostfixExpr::~PostfixExpr()
	{
		SAFE_DELETE(left);
	}
	STD_STRING PostfixExpr::ToString()
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

	STD_STRING ConditionExpr::ToString()
	{
		return condition->ToString() + TEXT("?") + trueBranch->ToString() + TEXT(":") + falseBranch->ToString();
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
	STD_STRING IndexExpr::ToString()
	{
		return ds->ToString() + TEXT("[") + index->ToString() + TEXT("]");
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

	STD_STRING RefExpr::ToString()
	{
		return TEXT("&") + refExpr->ToString();
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

	STD_STRING LambdaExpr::ToString()
	{
		STD_STRING result = TEXT("fn(");
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT(")");
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
	STD_STRING CallExpr::ToString()
	{
		STD_STRING result = callee->ToString() + TEXT("(");

		if (!arguments.empty())
		{
			for (const auto &arg : arguments)
				result += arg->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT(")");
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

	STD_STRING DotExpr::ToString()
	{
		return callee->ToString() + TEXT(".") + callMember->ToString();
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

	STD_STRING NewExpr::ToString()
	{
		return TEXT("new ") + callee->ToString();
	}

	ThisExpr::ThisExpr(Token *tagToken)
		: Expr(tagToken, AstKind::THIS)
	{
	}

	ThisExpr::~ThisExpr()
	{
	}

	STD_STRING ThisExpr::ToString()
	{
		return TEXT("this");
	}

	BaseExpr::BaseExpr(Token *tagToken, IdentifierExpr *callMember)
		: Expr(tagToken, AstKind::BASE), callMember(callMember)
	{
	}
	BaseExpr::~BaseExpr()
	{
		SAFE_DELETE(callMember);
	}

	STD_STRING BaseExpr::ToString()
	{
		return TEXT("base.") + callMember->ToString();
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

	STD_STRING CompoundExpr::ToString()
	{
		STD_STRING result = TEXT("({");
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		result += endExpr->ToString();
		result += TEXT("})");
		return result;
	}

	StructExpr::StructExpr(Token *tagToken)
		: Expr(tagToken, AstKind::STRUCT)
	{
	}
	StructExpr::StructExpr(Token *tagToken, const std::vector<std::pair<STD_STRING, Expr *>> &elements)
		: Expr(tagToken, AstKind::STRUCT), elements(elements)
	{
	}
	StructExpr::~StructExpr()
	{
		std::vector<std::pair<STD_STRING, Expr *>>().swap(elements);
	}

	STD_STRING StructExpr::ToString()
	{
		STD_STRING result = TEXT("{");
		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key + TEXT(":") + value->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("}");
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

	STD_STRING VarArgExpr::ToString()
	{
		return TEXT("...") + (argName ? argName->ToString() : TEXT(""));
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

	STD_STRING FactorialExpr::ToString()
	{
		return expr->ToString() + TEXT("!");
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

	STD_STRING AppregateExpr::ToString()
	{
		STD_STRING result = TEXT("(");
		if (!exprs.empty())
		{
			for (const auto &expr : exprs)
				result += expr->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}

		return result + TEXT(")");
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

	STD_STRING ExprStmt::ToString()
	{
		return expr->ToString() + TEXT(";");
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

	STD_STRING ReturnStmt::ToString()
	{
		if (!expr)
			return TEXT("return;");
		else
			return TEXT("return ") + expr->ToString() + TEXT(";");
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

	STD_STRING IfStmt::ToString()
	{
		STD_STRING result;
		result = TEXT("if(") + condition->ToString() + TEXT(")") + thenBranch->ToString();
		if (elseBranch != nullptr)
			result += TEXT("else ") + elseBranch->ToString();
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

	STD_STRING ScopeStmt::ToString()
	{
		STD_STRING result = TEXT("{");
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		result += TEXT("}");
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

	STD_STRING WhileStmt::ToString()
	{
		STD_STRING result = TEXT("while(") + condition->ToString() + TEXT("){") + body->ToString();
		if (increment)
			result += increment->ToString();
		return result += TEXT("}");
	}

	BreakStmt::BreakStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::BREAK)
	{
	}
	BreakStmt::~BreakStmt()
	{
	}

	STD_STRING BreakStmt::ToString()
	{
		return TEXT("break;");
	}

	ContinueStmt::ContinueStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::CONTINUE)
	{
	}
	ContinueStmt::~ContinueStmt()
	{
	}

	STD_STRING ContinueStmt::ToString()
	{
		return TEXT("continue;");
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

	STD_STRING AstStmts::ToString()
	{
		STD_STRING result;
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		return result;
	}

	//----------------------Declarations-----------------------------

	VarDecl::VarDecl(Token *tagToken)
		: Decl(tagToken, AstKind::VAR), privilege(Privilege::MUTABLE)
	{
	}
	VarDecl::VarDecl(Token *tagToken, Privilege privilege, const std::vector<std::pair<Expr *, Expr *>> &variables)
		: Decl(tagToken, AstKind::VAR), privilege(privilege), variables(variables)
	{
	}
	VarDecl::~VarDecl()
	{
		std::vector<std::pair<Expr *, Expr *>>().swap(variables);
	}

	STD_STRING VarDecl::ToString()
	{
		STD_STRING result;

		if (privilege == Privilege::IMMUTABLE)
			result += TEXT("const ");
		else
			result += TEXT("let ");

		if (!variables.empty())
		{
			for (auto [key, value] : variables)
				result += key->ToString() + TEXT("=") + value->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		return result + TEXT(";");
	}

	EnumDecl::EnumDecl(Token *tagToken)
		: Decl(tagToken, AstKind::ENUM), name(nullptr)
	{
	}
	EnumDecl::EnumDecl(Token *tagToken, IdentifierExpr *name, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems)
		: Decl(tagToken, AstKind::ENUM), name(name), enumItems(enumItems)
	{
	}
	EnumDecl::~EnumDecl()
	{
		SAFE_DELETE(name);
		std::unordered_map<IdentifierExpr *, Expr *>().swap(enumItems);
	}
	STD_STRING EnumDecl::ToString()
	{
		STD_STRING result = TEXT("enum ") + name->ToString() + TEXT("{");

		if (!enumItems.empty())
		{
			for (auto [key, value] : enumItems)
				result += key->ToString() + TEXT("=") + value->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		return result + TEXT("}");
	}

	ModuleDecl::ModuleDecl(Token *tagToken)
		: Decl(tagToken, AstKind::MODULE), name(nullptr)
	{
	}
	ModuleDecl::ModuleDecl(Token *tagToken,
						   IdentifierExpr *name,
						   const std::vector<VarDecl *> &varItems,
						   const std::vector<ClassDecl *> &classItems,
						   const std::vector<ModuleDecl *> &moduleItems,
						   const std::vector<EnumDecl *> &enumItems,
						   const std::vector<FunctionDecl *> &functionItems)
		: Decl(tagToken, AstKind::MODULE), name(name),
		  varItems(varItems),
		  classItems(classItems),
		  moduleItems(moduleItems),
		  enumItems(enumItems),
		  functionItems(functionItems)
	{
	}
	ModuleDecl::~ModuleDecl()
	{
		SAFE_DELETE(name);
		std::vector<VarDecl *>().swap(varItems);
		std::vector<ClassDecl *>().swap(classItems);
		std::vector<ModuleDecl *>().swap(moduleItems);
		std::vector<EnumDecl *>().swap(enumItems);
		std::vector<FunctionDecl *>().swap(functionItems);
	}

	STD_STRING ModuleDecl::ToString()
	{
		STD_STRING result = TEXT("module ") + name->ToString() + TEXT("\n{\n");
		for (const auto &item : varItems)
			result += item->ToString() + TEXT("\n");
		for (const auto &item : classItems)
			result += item->ToString() + TEXT("\n");
		for (const auto &item : moduleItems)
			result += item->ToString() + TEXT("\n");
		for (const auto &item : enumItems)
			result += item->ToString() + TEXT("\n");
		for (const auto &item : functionItems)
			result += item->ToString() + TEXT("\n");
		return result + TEXT("}\n");
	}

	FunctionDecl::FunctionDecl(Token *tagToken)
		: Decl(tagToken, AstKind::FUNCTION), name(nullptr), body(nullptr), functionKind(FunctionKind::FUNCTION)
	{
	}

	FunctionDecl::FunctionDecl(Token *tagToken, FunctionKind kind, IdentifierExpr *name, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body)
		: Decl(tagToken, AstKind::FUNCTION), functionKind(kind), name(name), parameters(parameters), body(body)
	{
	}

	FunctionDecl::~FunctionDecl()
	{
		SAFE_DELETE(name);
		std::vector<VarDescExpr *>().swap(parameters);
		SAFE_DELETE(body);
	}

	STD_STRING FunctionDecl::ToString()
	{
		STD_STRING result = TEXT("fn ") + name->ToString() + TEXT("(");
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT(")");
		result += body->ToString();
		return result;
	}

	ClassDecl::ClassDecl(Token *tagToken)
		: Decl(tagToken, AstKind::CLASS)
	{
	}
	ClassDecl::ClassDecl(Token *tagToken,
						 STD_STRING name,
						 const std::vector<VarDecl *> &varItems,
						 const std::vector<FunctionDecl *> &fnItems,
						 const std::vector<EnumDecl *> &enumItems,
						 const std::vector<FunctionDecl *> &constructors,
						 const std::vector<IdentifierExpr *> &parentClasses)
		: Decl(tagToken, AstKind::CLASS),
		  name(name),
		  varItems(varItems),
		  fnItems(fnItems),
		  enumItems(enumItems),
		  constructors(constructors),
		  parentClasses(parentClasses)
	{
	}
	ClassDecl::~ClassDecl()
	{
		std::vector<IdentifierExpr *>().swap(parentClasses);
		std::vector<FunctionDecl *>().swap(constructors);
		std::vector<VarDecl *>().swap(varItems);
		std::vector<FunctionDecl *>().swap(fnItems);
		std::vector<EnumDecl *>().swap(enumItems);
	}

	STD_STRING ClassDecl::ToString()
	{
		STD_STRING result = TEXT("class ") + name;
		if (!parentClasses.empty())
		{
			result += TEXT(":");
			for (const auto &parentClass : parentClasses)
				result += parentClass->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("{");
		for (auto enumStmt : enumItems)
			result += enumStmt->ToString();
		for (auto variableStmt : varItems)
			result += variableStmt->ToString();
		for (auto fnStmt : fnItems)
			result += fnStmt->ToString();
		return result + TEXT("}");
	}
}
