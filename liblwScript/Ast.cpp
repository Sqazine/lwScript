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

	std::vector<Expr *> IntNumExpr::GetPostfixExpr()
	{
		return {};
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

	std::vector<Expr *> RealNumExpr::GetPostfixExpr()
	{
		return {};
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

	std::vector<Expr *> StrExpr::GetPostfixExpr()
	{
		return {};
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

	std::vector<Expr *> NullExpr::GetPostfixExpr()
	{
		return {};
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

	std::vector<Expr *> BoolExpr::GetPostfixExpr()
	{
		return {};
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

	std::vector<Expr *> IdentifierExpr::GetPostfixExpr()
	{
		return {};
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

	std::vector<Expr *> ArrayExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		for (const auto &e : elements)
		{
			auto eResult = e->GetPostfixExpr();
			result.insert(result.end(), eResult.begin(), eResult.end());
		}
		return result;
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

	std::vector<Expr *> TableExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		for (const auto &[k, v] : elements)
		{
			auto kResult = k->GetPostfixExpr();
			auto vResult = v->GetPostfixExpr();
			result.insert(result.end(), kResult.begin(), kResult.end());
			result.insert(result.end(), vResult.begin(), vResult.end());
		}
		return result;
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

	std::vector<Expr *> GroupExpr::GetPostfixExpr()
	{
		return expr->GetPostfixExpr();
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

	std::vector<Expr *> PrefixExpr::GetPostfixExpr()
	{
		return right->GetPostfixExpr();
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

	std::vector<Expr *> InfixExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		auto leftResult = left->GetPostfixExpr();
		auto rightResult = right->GetPostfixExpr();

		result.insert(result.end(), leftResult.begin(), leftResult.end());
		result.insert(result.end(), rightResult.begin(), rightResult.end());
		return result;
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

	std::vector<Expr *> ConditionExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		auto conditionResult = condition->GetPostfixExpr();
		result.insert(result.end(), conditionResult.begin(), conditionResult.end());
		auto trueBranchResult = trueBranch->GetPostfixExpr();
		result.insert(result.end(), trueBranchResult.begin(), trueBranchResult.end());
		auto falseBranchResult = falseBranch->GetPostfixExpr();
		result.insert(result.end(), falseBranchResult.begin(), falseBranchResult.end());
		return result;
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

	std::vector<Expr *> IndexExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		auto dsResult = ds->GetPostfixExpr();
		result.insert(result.end(), dsResult.begin(), dsResult.end());
		auto indexResult = index->GetPostfixExpr();
		result.insert(result.end(), indexResult.begin(), indexResult.end());
		return result;
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

	std::vector<Expr *> RefExpr::GetPostfixExpr()
	{
		return refExpr->GetPostfixExpr();
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

	std::vector<Expr *> LambdaExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		for (const auto &param : parameters)
		{
			auto paramResult = param->GetPostfixExpr();
			result.insert(result.end(), paramResult.begin(), paramResult.end());
		}
		auto bodyResult = body->GetPostfixExpr();
		result.insert(result.end(), bodyResult.begin(), bodyResult.end());
		return result;
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

	std::vector<Expr *> FunctionCallExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		auto nameResult = name->GetPostfixExpr();
		result.insert(result.end(), nameResult.begin(), nameResult.end());
		for (const auto &argument : arguments)
		{
			auto argumentResult = argument->GetPostfixExpr();
			result.insert(result.end(), argumentResult.begin(), argumentResult.end());
		}
		return result;
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

	std::vector<Expr *> FieldCallExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		auto calleeResult = callee->GetPostfixExpr();
		result.insert(result.end(), calleeResult.begin(), calleeResult.end());
		auto callMemberResult = callMember->GetPostfixExpr();
		result.insert(result.end(), callMemberResult.begin(), callMemberResult.end());
		return result;
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

	std::vector<Expr *> ExprStmt::GetPostfixExpr()
	{
		return expr->GetPostfixExpr();
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

	std::vector<Expr *> LetStmt::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		for (const auto &[k, v] : variables)
		{
			auto varResult = v->GetPostfixExpr();
			result.insert(result.end(), varResult.begin(), varResult.end());
		}
		return result;
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

	std::vector<Expr *> ConstStmt::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		for (const auto &[k, v] : consts)
		{
			auto varResult = v->GetPostfixExpr();
			result.insert(result.end(), varResult.begin(), varResult.end());
		}
		return result;
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

	std::vector<Expr *> ReturnStmt::GetPostfixExpr()
	{
		return expr->GetPostfixExpr();
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

	std::vector<Expr *> IfStmt::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		auto conditionResult = condition->GetPostfixExpr();
		result.insert(result.end(), conditionResult.begin(), conditionResult.end());
		auto thenBranchResult = thenBranch->GetPostfixExpr();
		result.insert(result.end(), thenBranchResult.begin(), thenBranchResult.end());
		auto elseBranchResult = elseBranch->GetPostfixExpr();
		result.insert(result.end(), elseBranchResult.begin(), elseBranchResult.end());
		return result;
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

	std::vector<Expr *> ScopeStmt::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		for (const auto &stmt : stmts)
		{
			auto stmtResult = stmt->GetPostfixExpr();
			result.insert(result.end(), stmtResult.begin(), stmtResult.end());
		}
		return result;
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

	std::vector<Expr *> WhileStmt::GetPostfixExpr()
	{
		std::vector<Expr *> result = condition->GetPostfixExpr();
		auto bodyResult = body->GetPostfixExpr();
		result.insert(result.end(), bodyResult.begin(), bodyResult.end());
		auto incrementResult = increment->GetPostfixExpr();
		result.insert(result.end(), incrementResult.begin(), incrementResult.end());
		return result;
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

	std::vector<Expr *> BreakStmt::GetPostfixExpr()
	{
		return {};
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

	std::vector<Expr *> ContinueStmt::GetPostfixExpr()
	{
		return {};
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

	std::vector<Expr *> EnumStmt::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		for (const auto &[k, v] : enumItems)
		{
			auto kResult = k->GetPostfixExpr();
			auto vResult = v->GetPostfixExpr();
			result.insert(result.end(), kResult.begin(), kResult.end());
			result.insert(result.end(), vResult.begin(), vResult.end());
		}
		return result;
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

	std::vector<Expr *> FunctionStmt::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		auto bodyResult = body->GetPostfixExpr();
		result.insert(result.end(), bodyResult.begin(), bodyResult.end());
		return result;
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

	std::vector<Expr *> FieldStmt::GetPostfixExpr()
	{
		std::vector<Expr *> result;

		for (const auto &letStmt : letStmts)
		{
			auto letStmtResult = letStmt->GetPostfixExpr();
			result.insert(result.end(), letStmtResult.begin(), letStmtResult.end());
		}

		for (const auto &constStmt : constStmts)
		{
			auto constStmtResult = constStmt->GetPostfixExpr();
			result.insert(result.end(), constStmtResult.begin(), constStmtResult.end());
		}

		for (const auto &fnStmt : fnStmts)
		{
			auto fnStmtResult = fnStmt->GetPostfixExpr();
			result.insert(result.end(), fnStmtResult.begin(), fnStmtResult.end());
		}
		return result;
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
	std::vector<Expr *> AstStmts::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		for (const auto &stmt : stmts)
		{
			auto stmtResult = stmt->GetPostfixExpr();
			result.insert(result.end(), stmtResult.begin(), stmtResult.end());
		}
		return result;
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
	std::vector<Expr *> PostfixExpr::GetPostfixExpr()
	{
		std::vector<Expr *> result;
		auto leftResult = left->GetPostfixExpr();
		result.insert(result.end(), leftResult.begin(), leftResult.end());
		result.emplace_back(this);
		return result;
	}
}
