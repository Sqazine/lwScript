#include "ConstantFolder.h"
#include "Utils.h"
namespace lws
{
	ConstantFolder::ConstantFolder()
	{
	}
	ConstantFolder::~ConstantFolder()
	{
	}

	Stmt *ConstantFolder::Fold(Stmt *stmt)
	{
		return FoldStmt(stmt);
	}

	Stmt *ConstantFolder::FoldStmt(Stmt *stmt)
	{
		switch (stmt->Type())
		{
		case AST_ASTSTMTS:
			return FoldAstStmts((AstStmts *)stmt);
		case AST_RETURN:
			return FoldReturnStmt((ReturnStmt *)stmt);
		case AST_EXPR:
			return FoldExprStmt((ExprStmt *)stmt);
		case AST_LET:
			return FoldLetStmt((LetStmt *)stmt);
		case AST_CONST:
			return FoldConstStmt((ConstStmt *)stmt);
		case AST_SCOPE:
			return FoldScopeStmt((ScopeStmt *)stmt);
		case AST_IF:
			return FoldIfStmt((IfStmt *)stmt);
		case AST_WHILE:
			return FoldWhileStmt((WhileStmt *)stmt);
		case AST_FUNCTION:
			return FoldFunctionStmt((FunctionStmt *)stmt);
		default:
			return stmt;
		}
	}
	Stmt *ConstantFolder::FoldAstStmts(AstStmts *stmt)
	{
		for (auto &s : stmt->stmts)
			s = FoldStmt(s);
		return stmt;
	}
	Stmt *ConstantFolder::FoldExprStmt(ExprStmt *stmt)
	{
		stmt->expr = FoldExpr(stmt->expr);
		return stmt;
	}
	Stmt *ConstantFolder::FoldIfStmt(IfStmt *stmt)
	{
		stmt->condition = FoldExpr(stmt->condition);
		stmt->thenBranch = FoldStmt(stmt->thenBranch);
		if (stmt->elseBranch)
			stmt->elseBranch = FoldStmt(stmt->elseBranch);

		if (stmt->condition->Type() == AST_BOOL)
		{
			if (((BoolExpr *)stmt->condition)->value == true)
				return stmt->thenBranch;
			else
				return stmt->elseBranch;
		}

		return stmt;
	}
	Stmt *ConstantFolder::FoldScopeStmt(ScopeStmt *stmt)
	{
		for (auto &s : stmt->stmts)
			s = FoldStmt(s);
		return stmt;
	}
	Stmt *ConstantFolder::FoldWhileStmt(WhileStmt *stmt)
	{
		stmt->condition = FoldExpr(stmt->condition);
		stmt->body = (ScopeStmt *)FoldScopeStmt(stmt->body);
		return stmt;
	}
	Stmt *ConstantFolder::FoldEnumStmt(EnumStmt *stmt)
	{
		for (auto &[k, v] : stmt->enumItems)
			v = FoldExpr(v);
		return stmt;
	}
	Stmt *ConstantFolder::FoldReturnStmt(ReturnStmt *stmt)
	{
		if (stmt->expr)
			stmt->expr = FoldExpr(stmt->expr);
		return stmt;
	}
	Stmt *ConstantFolder::FoldLetStmt(LetStmt *stmt)
	{
		for (auto &[k, v] : stmt->variables)
			v.value = FoldExpr(v.value);
		return stmt;
	}
	Stmt *ConstantFolder::FoldConstStmt(ConstStmt *stmt)
	{
		for (auto &[k, v] : stmt->consts)
			v.value = FoldExpr(v.value);
		return stmt;
	}
	Stmt *ConstantFolder::FoldFunctionStmt(FunctionStmt *stmt)
	{
		for (auto &e : stmt->parameters)
			e = (IdentifierExpr *)FoldIdentifierExpr(e);

		stmt->body = (ScopeStmt *)FoldScopeStmt(stmt->body);
		return stmt;
	}
	Stmt *ConstantFolder::FoldClassStmt(ClassStmt *stmt)
	{
		for (auto &letStmt : stmt->letStmts)
			letStmt = (LetStmt *)FoldLetStmt(letStmt);

		for (auto &constStmt : stmt->constStmts)
			constStmt = (ConstStmt *)FoldConstStmt(constStmt);

		for (auto &fnStmt : stmt->fnStmts)
			fnStmt = (FunctionStmt *)FoldFunctionStmt(fnStmt);

		return stmt;
	}

	Expr *ConstantFolder::FoldExpr(Expr *expr)
	{
		switch (expr->Type())
		{
		case AST_INT:
			return FoldIntNumExpr((IntNumExpr *)expr);
		case AST_REAL:
			return FoldRealNumExpr((RealNumExpr *)expr);
		case AST_STR:
			return FoldStrExpr((StrExpr *)expr);
		case AST_BOOL:
			return FoldBoolExpr((BoolExpr *)expr);
		case AST_NULL:
			return FoldNullExpr((NullExpr *)expr);
		case AST_IDENTIFIER:
			return FoldIdentifierExpr((IdentifierExpr *)expr);
		case AST_GROUP:
			return FoldGroupExpr((GroupExpr *)expr);
		case AST_ARRAY:
			return FoldArrayExpr((ArrayExpr *)expr);
		case AST_INDEX:
			return FoldIndexExpr((IndexExpr *)expr);
		case AST_PREFIX:
			return FoldPrefixExpr((PrefixExpr *)expr);
		case AST_INFIX:
			return FoldInfixExpr((InfixExpr *)expr);
		case AST_POSTFIX:
			return FoldPostfixExpr((PostfixExpr *)expr);
		case AST_CONDITION:
			return FoldConditionExpr((ConditionExpr *)expr);
		case AST_REF:
			return FoldRefExpr((RefExpr *)expr);
		case AST_CALL:
			return FoldCallExpr((CallExpr *)expr);
		case AST_DOT:
			return FoldDotExpr((DotExpr *)expr);
		case AST_LAMBDA:
			return FoldLambdaExpr((LambdaExpr *)expr);
		default:
			return expr;
		}
	}
	Expr *ConstantFolder::FoldInfixExpr(InfixExpr *expr)
	{
		expr->left = FoldExpr(expr->left);
		expr->right = FoldExpr(expr->right);

		return ConstantFold(expr);
	}
	Expr *ConstantFolder::FoldPostfixExpr(PostfixExpr *expr)
	{
		expr->left = FoldExpr(expr->left);
		return ConstantFold(expr);
	}
	Expr *ConstantFolder::FoldConditionExpr(ConditionExpr *expr)
	{
		expr->condition = FoldExpr(expr->condition);
		expr->trueBranch = FoldExpr(expr->trueBranch);
		expr->falseBranch = FoldExpr(expr->falseBranch);

		if (expr->condition->Type() == AST_BOOL)
		{
			auto boolExpr = (BoolExpr *)expr->condition;
			if (boolExpr->value)
				return expr->trueBranch;
			else
				return expr->falseBranch;
		}

		return expr;
	}
	Expr *ConstantFolder::FoldIntNumExpr(IntNumExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldRealNumExpr(RealNumExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldBoolExpr(BoolExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldPrefixExpr(PrefixExpr *expr)
	{
		expr->right = FoldExpr(expr->right);
		return ConstantFold(expr);
	}
	Expr *ConstantFolder::FoldStrExpr(StrExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldNullExpr(NullExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldGroupExpr(GroupExpr *expr)
	{
		return FoldExpr(expr->expr);
	}
	Expr *ConstantFolder::FoldArrayExpr(ArrayExpr *expr)
	{
		for (auto &e : expr->elements)
			e = FoldExpr(e);
		return expr;
	}
	Expr *ConstantFolder::FoldTableExpr(TableExpr *expr)
	{
		for (auto &[k, v] : expr->elements)
		{
			k = FoldExpr(k);
			v = FoldExpr(v);
		}
		return expr;
	}
	Expr *ConstantFolder::FoldIndexExpr(IndexExpr *expr)
	{
		expr->ds = FoldExpr(expr->ds);
		expr->index = FoldExpr(expr->index);
		return expr;
	}
	Expr *ConstantFolder::FoldIdentifierExpr(IdentifierExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldLambdaExpr(LambdaExpr *expr)
	{
		for (auto &e : expr->parameters)
			e = (IdentifierExpr *)FoldIdentifierExpr(e);
		expr->body = (ScopeStmt *)FoldScopeStmt(expr->body);
		return expr;
	}
	Expr *ConstantFolder::FoldDotExpr(DotExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldCallExpr(CallExpr *expr)
	{
		expr->callee = FoldExpr(expr->callee);
		for (auto &arg : expr->arguments)
			arg = FoldExpr(arg);
		return expr;
	}
	Expr *ConstantFolder::FoldNewExpr(NewExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldThisExpr(ThisExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldBaseExpr(BaseExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFolder::FoldRefExpr(RefExpr *expr)
	{
		expr->refExpr = (IdentifierExpr *)FoldExpr(expr->refExpr);
		return expr;
	}

	Expr *ConstantFolder::ConstantFold(Expr *expr)
	{
		if (expr->Type() == AST_INFIX)
		{
			auto infix = (InfixExpr *)expr;
			if (infix->left->Type() == AST_REAL && infix->right->Type() == AST_REAL)
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
			else if (infix->left->Type() == AST_INT && infix->right->Type() == AST_INT)
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
			else if (infix->left->Type() == AST_INT && infix->right->Type() == AST_REAL)
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
			else if (infix->left->Type() == AST_REAL && infix->right->Type() == AST_INT)
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
			else if (infix->left->Type() == AST_STR && infix->right->Type() == AST_STR)
			{
				auto strExpr = new StrExpr(((StrExpr *)infix->left)->value + ((StrExpr *)infix->right)->value);
				delete infix;
				infix = nullptr;
				return strExpr;
			}
		}
		else if (expr->Type() == AST_PREFIX)
		{
			auto prefix = (PrefixExpr *)expr;
			if (prefix->right->Type() == AST_REAL && prefix->op == L"-")
			{
				auto numExpr = new RealNumExpr(-((RealNumExpr *)prefix->right)->value);
				delete prefix;
				prefix = nullptr;
				return numExpr;
			}
			else if (prefix->right->Type() == AST_BOOL && prefix->op == L"!")
			{
				auto boolExpr = new BoolExpr(!((BoolExpr *)prefix->right)->value);
				delete prefix;
				prefix = nullptr;
				return boolExpr;
			}
		}
		else if (expr->Type() == AST_POSTFIX)
		{
			auto postfix = (PostfixExpr *)expr;
			if (postfix->left->Type() == AST_INT && postfix->op == L"!")
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