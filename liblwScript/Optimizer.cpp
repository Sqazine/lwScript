#include "Optimizer.h"
#include "Utils.h"
namespace lws
{
	Optimizer::Optimizer()
	{
	}
	Optimizer::~Optimizer()
	{
	}

	Stmt *Optimizer::Opt(Stmt *stmt)
	{
		return OptStmt(stmt);
	}

	Stmt *Optimizer::OptStmt(Stmt *stmt)
	{
		switch (stmt->type)
		{
		case AST_ASTSTMTS:
			return OptAstStmts((AstStmts *)stmt);
		case AST_RETURN:
			return OptReturnStmt((ReturnStmt *)stmt);
		case AST_EXPR:
			return OptExprStmt((ExprStmt *)stmt);
		case AST_VAR:
			return OptVarStmt((VarStmt *)stmt);
		case AST_SCOPE:
			return OptScopeStmt((ScopeStmt *)stmt);
		case AST_IF:
			return OptIfStmt((IfStmt *)stmt);
		case AST_WHILE:
			return OptWhileStmt((WhileStmt *)stmt);
		case AST_FUNCTION:
			return OptFunctionStmt((FunctionStmt *)stmt);
		case AST_CLASS:
			return OptClassStmt((ClassStmt *)stmt);
		default:
			return stmt;
		}
	}
	Stmt *Optimizer::OptAstStmts(AstStmts *stmt)
	{
		for (auto &s : stmt->stmts)
			s = OptStmt(s);
		return stmt;
	}
	Stmt *Optimizer::OptExprStmt(ExprStmt *stmt)
	{
		stmt->expr = OptExpr(stmt->expr);
		return stmt;
	}
	Stmt *Optimizer::OptIfStmt(IfStmt *stmt)
	{
		stmt->condition = OptExpr(stmt->condition);
		stmt->thenBranch = OptStmt(stmt->thenBranch);
		if (stmt->elseBranch)
			stmt->elseBranch = OptStmt(stmt->elseBranch);

		if (stmt->condition->type == AST_BOOL)
		{
			if (((BoolExpr *)stmt->condition)->value == true)
				return stmt->thenBranch;
			else
				return stmt->elseBranch;
		}

		return stmt;
	}
	Stmt *Optimizer::OptScopeStmt(ScopeStmt *stmt)
	{
		for (auto &s : stmt->stmts)
			s = OptStmt(s);
		return stmt;
	}
	Stmt *Optimizer::OptWhileStmt(WhileStmt *stmt)
	{
		stmt->condition = OptExpr(stmt->condition);
		stmt->body = (ScopeStmt *)OptScopeStmt(stmt->body);
		return stmt;
	}
	Stmt *Optimizer::OptEnumStmt(EnumStmt *stmt)
	{
		for (auto &[k, v] : stmt->enumItems)
			v = OptExpr(v);
		return stmt;
	}
	Stmt *Optimizer::OptReturnStmt(ReturnStmt *stmt)
	{
		return stmt;
	}
	Stmt *Optimizer::OptVarStmt(VarStmt *stmt)
	{
		for (auto &[k, v] : stmt->variables)
			v = OptExpr(v);
		return stmt;
	}

	Stmt *Optimizer::OptFunctionStmt(FunctionStmt *stmt)
	{
		for (auto &e : stmt->parameters)
			e = (VarDescExpr *)OptVarDescExpr(e);

		stmt->body = (ScopeStmt *)OptScopeStmt(stmt->body);

		return stmt;
	}
	Stmt *Optimizer::OptClassStmt(ClassStmt *stmt)
	{
		for (auto &varStmt : stmt->varStmts)
			varStmt = (VarStmt *)OptVarStmt(varStmt);

		for (auto &fnStmt : stmt->fnStmts)
			fnStmt = (FunctionStmt *)OptFunctionStmt(fnStmt);

		return stmt;
	}

	Expr *Optimizer::OptExpr(Expr *expr)
	{
		switch (expr->type)
		{
		case AST_INT:
			return OptIntNumExpr((IntNumExpr *)expr);
		case AST_REAL:
			return OptRealNumExpr((RealNumExpr *)expr);
		case AST_STR:
			return OptStrExpr((StrExpr *)expr);
		case AST_BOOL:
			return OptBoolExpr((BoolExpr *)expr);
		case AST_NULL:
			return OptNullExpr((NullExpr *)expr);
		case AST_IDENTIFIER:
			return OptIdentifierExpr((IdentifierExpr *)expr);
		case AST_GROUP:
			return OptGroupExpr((GroupExpr *)expr);
		case AST_ARRAY:
			return OptArrayExpr((ArrayExpr *)expr);
		case AST_INDEX:
			return OptIndexExpr((IndexExpr *)expr);
		case AST_PREFIX:
			return OptPrefixExpr((PrefixExpr *)expr);
		case AST_INFIX:
			return OptInfixExpr((InfixExpr *)expr);
		case AST_POSTFIX:
			return OptPostfixExpr((PostfixExpr *)expr);
		case AST_CONDITION:
			return OptConditionExpr((ConditionExpr *)expr);
		case AST_REF:
			return OptRefExpr((RefExpr *)expr);
		case AST_CALL:
			return OptCallExpr((CallExpr *)expr);
		case AST_DOT:
			return OptDotExpr((DotExpr *)expr);
		case AST_LAMBDA:
			return OptLambdaExpr((LambdaExpr *)expr);
		case AST_FACTORIAL:
			return OptFactorialExpr((FactorialExpr *)expr);
		default:
			return expr;
		}
	}
	Expr *Optimizer::OptInfixExpr(InfixExpr *expr)
	{
		expr->left = OptExpr(expr->left);
		expr->right = OptExpr(expr->right);

		return OptFlow(expr);
	}
	Expr *Optimizer::OptPostfixExpr(PostfixExpr *expr)
	{
		expr->left = OptExpr(expr->left);
		return OptFlow(expr);
	}
	Expr *Optimizer::OptConditionExpr(ConditionExpr *expr)
	{
		expr->condition = OptExpr(expr->condition);
		expr->trueBranch = OptExpr(expr->trueBranch);
		expr->falseBranch = OptExpr(expr->falseBranch);

		if (expr->condition->type == AST_BOOL)
		{
			auto boolExpr = (BoolExpr *)expr->condition;
			if (boolExpr->value)
				return expr->trueBranch;
			else
				return expr->falseBranch;
		}

		return expr;
	}
	Expr *Optimizer::OptIntNumExpr(IntNumExpr *expr)
	{
		return expr;
	}
	Expr *Optimizer::OptRealNumExpr(RealNumExpr *expr)
	{
		return expr;
	}
	Expr *Optimizer::OptBoolExpr(BoolExpr *expr)
	{
		return expr;
	}
	Expr *Optimizer::OptPrefixExpr(PrefixExpr *expr)
	{
		expr->right = OptExpr(expr->right);
		return OptFlow(expr);
	}
	Expr *Optimizer::OptStrExpr(StrExpr *expr)
	{
		return expr;
	}
	Expr *Optimizer::OptNullExpr(NullExpr *expr)
	{
		return expr;
	}
	Expr *Optimizer::OptGroupExpr(GroupExpr *expr)
	{
		return OptExpr(expr->expr);
	}
	Expr *Optimizer::OptArrayExpr(ArrayExpr *expr)
	{
		for (auto &e : expr->elements)
			e = OptExpr(e);
		return expr;
	}
	Expr *Optimizer::OptDictExpr(DictExpr *expr)
	{
		for (auto &[k, v] : expr->elements)
		{
			k = OptExpr(k);
			v = OptExpr(v);
		}
		return expr;
	}
	Expr *Optimizer::OptIndexExpr(IndexExpr *expr)
	{
		expr->ds = OptExpr(expr->ds);
		expr->index = OptExpr(expr->index);
		return expr;
	}
	Expr *Optimizer::OptIdentifierExpr(IdentifierExpr *expr)
	{
		return expr;
	}
	Expr *Optimizer::OptLambdaExpr(LambdaExpr *expr)
	{
		for (auto &e : expr->parameters)
			e = (VarDescExpr *)OptVarDescExpr(e);
		expr->body = (ScopeStmt *)OptScopeStmt(expr->body);
		return expr;
	}

	Expr *Optimizer::OptDotExpr(DotExpr *expr)
	{
		return expr;
	}

	Expr *Optimizer::OptCallExpr(CallExpr *expr)
	{
		expr->callee = OptExpr(expr->callee);
		for (auto &arg : expr->arguments)
			arg = OptExpr(arg);
		return expr;
	}
	Expr *Optimizer::OptNewExpr(NewExpr *expr)
	{
		return expr;
	}
	Expr *Optimizer::OptThisExpr(ThisExpr *expr)
	{
		return expr;
	}
	Expr *Optimizer::OptBaseExpr(BaseExpr *expr)
	{
		return expr;
	}

	Expr *Optimizer::OptFactorialExpr(FactorialExpr *expr)
	{
		if (expr->expr->type == AST_INT)
		{
			auto intExpr = new IntNumExpr();
			intExpr->value = Factorial(((IntNumExpr *)expr->expr)->value);

			delete expr;
			expr = nullptr;

			return intExpr;
		}
		else
		{
			expr->expr = OptExpr(expr->expr);
			return expr;
		}
	}

	Expr *Optimizer::OptVarDescExpr(VarDescExpr *expr)
	{
		return expr;
	}

	Expr *Optimizer::OptRefExpr(RefExpr *expr)
	{
		expr->refExpr = (IdentifierExpr *)OptExpr(expr->refExpr);
		return expr;
	}

	Expr *Optimizer::OptFlow(Expr *expr)
	{
		expr = ConstantFold(expr);
		return expr;
	}

	Expr *Optimizer::ConstantFold(Expr *expr)
	{
		if (expr->type == AST_INFIX)
		{
			auto infix = (InfixExpr *)expr;
			if (infix->left->type == AST_REAL && infix->right->type == AST_REAL)
			{
				Expr *newExpr = nullptr;
				if (infix->op == L"+")
					newExpr = new RealNumExpr(((RealNumExpr *)infix->left)->value + ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"-")
					newExpr = new RealNumExpr(((RealNumExpr *)infix->left)->value - ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"*")
					newExpr = new RealNumExpr(((RealNumExpr *)infix->left)->value * ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"/")
					newExpr = new RealNumExpr(((RealNumExpr *)infix->left)->value / ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"==")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value == ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"!=")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value != ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L">")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value > ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L">=")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value >= ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"<")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value < ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"<=")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value <= ((RealNumExpr *)infix->right)->value);

				delete infix;
				infix = nullptr;
				return newExpr;
			}
			else if (infix->left->type == AST_INT && infix->right->type == AST_INT)
			{
				Expr *newExpr = nullptr;
				if (infix->op == L"+")
					newExpr = new IntNumExpr(((IntNumExpr *)infix->left)->value + ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"-")
					newExpr = new IntNumExpr(((IntNumExpr *)infix->left)->value - ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"*")
					newExpr = new IntNumExpr(((IntNumExpr *)infix->left)->value * ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"/")
					newExpr = new IntNumExpr(((IntNumExpr *)infix->left)->value / ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"%")
					newExpr = new IntNumExpr(((IntNumExpr *)infix->left)->value % ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"&")
					newExpr = new IntNumExpr(((IntNumExpr *)infix->left)->value & ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"|")
					newExpr = new IntNumExpr(((IntNumExpr *)infix->left)->value | ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"^")
					newExpr = new IntNumExpr(((IntNumExpr *)infix->left)->value ^ ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"==")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value == ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"!=")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value != ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L">")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value > ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L">=")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value >= ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"<")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value < ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"<=")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value <= ((IntNumExpr *)infix->right)->value);

				delete infix;
				infix = nullptr;
				return newExpr;
			}
			else if (infix->left->type == AST_INT && infix->right->type == AST_REAL)
			{
				Expr *newExpr = nullptr;
				if (infix->op == L"+")
					newExpr = new RealNumExpr(((IntNumExpr *)infix->left)->value + ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"-")
					newExpr = new RealNumExpr(((IntNumExpr *)infix->left)->value - ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"*")
					newExpr = new RealNumExpr(((IntNumExpr *)infix->left)->value * ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"/")
					newExpr = new RealNumExpr(((IntNumExpr *)infix->left)->value / ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"==")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value == ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"!=")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value != ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L">")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value > ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L">=")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value >= ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"<")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value < ((RealNumExpr *)infix->right)->value);
				else if (infix->op == L"<=")
					newExpr = new BoolExpr(((IntNumExpr *)infix->left)->value <= ((RealNumExpr *)infix->right)->value);

				delete infix;
				infix = nullptr;
				return newExpr;
			}
			else if (infix->left->type == AST_REAL && infix->right->type == AST_INT)
			{
				Expr *newExpr = nullptr;
				if (infix->op == L"+")
					newExpr = new RealNumExpr(((RealNumExpr *)infix->left)->value + ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"-")
					newExpr = new RealNumExpr(((RealNumExpr *)infix->left)->value - ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"*")
					newExpr = new RealNumExpr(((RealNumExpr *)infix->left)->value * ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"/")
					newExpr = new RealNumExpr(((RealNumExpr *)infix->left)->value / ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"==")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value == ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"!=")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value != ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L">")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value > ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L">=")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value >= ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"<")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value < ((IntNumExpr *)infix->right)->value);
				else if (infix->op == L"<=")
					newExpr = new BoolExpr(((RealNumExpr *)infix->left)->value <= ((IntNumExpr *)infix->right)->value);

				delete infix;
				infix = nullptr;
				return newExpr;
			}
			else if (infix->left->type == AST_STR && infix->right->type == AST_STR)
			{
				auto strExpr = new StrExpr(((StrExpr *)infix->left)->value + ((StrExpr *)infix->right)->value);
				delete infix;
				infix = nullptr;
				return strExpr;
			}
		}
		else if (expr->type == AST_PREFIX)
		{
			auto prefix = (PrefixExpr *)expr;
			if (prefix->right->type == AST_REAL && prefix->op == L"-")
			{
				auto numExpr = new RealNumExpr(-((RealNumExpr *)prefix->right)->value);
				delete prefix;
				prefix = nullptr;
				return numExpr;
			}
			else if (prefix->right->type == AST_BOOL && prefix->op == L"!")
			{
				auto boolExpr = new BoolExpr(!((BoolExpr *)prefix->right)->value);
				delete prefix;
				prefix = nullptr;
				return boolExpr;
			}
			else if (prefix->right->type == AST_INT && prefix->op == L"~")
			{
				auto newExpr = new IntNumExpr(~((IntNumExpr *)prefix->right)->value);
				delete prefix;
				prefix = nullptr;
				return newExpr;
			}
		}
		else if (expr->type == AST_POSTFIX)
		{
			auto postfix = (PostfixExpr *)expr;
			if (postfix->left->type == AST_INT && postfix->op == L"!")
			{
				auto numExpr = new IntNumExpr(Factorial(((IntNumExpr *)postfix->left)->value));
				delete postfix;
				postfix = nullptr;
				return numExpr;
			}
		}

		return expr;
	}
}