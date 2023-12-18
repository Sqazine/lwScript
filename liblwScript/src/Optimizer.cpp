#include "Optimizer.h"
#include "Utils.h"

namespace lwscript
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
		case AST_MODULE:
			return OptModuleStmt((ModuleStmt *)stmt);
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

		if (stmt->condition->type == AST_LITERAL && ((LiteralExpr *)stmt->condition)->literalType == LiteralExpr::Type::BOOLEAN)
		{
			if (((LiteralExpr *)stmt->condition)->boolean == true)
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
		for (auto &varStmt : stmt->varItems)
			varStmt = (VarStmt *)OptVarStmt(varStmt);

		for (auto &fnStmt : stmt->fnItems)
			fnStmt = (FunctionStmt *)OptFunctionStmt(fnStmt);

		return stmt;
	}

	Stmt *Optimizer::OptModuleStmt(ModuleStmt *stmt)
	{
		return stmt;
	}

	Expr *Optimizer::OptExpr(Expr *expr)
	{
		switch (expr->type)
		{
		case AST_LITERAL:
			return OptLiteralExpr((LiteralExpr *)expr);
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

		if (expr->condition->type == AST_LITERAL && ((LiteralExpr *)expr->condition)->literalType == LiteralExpr::Type::BOOLEAN)
		{
			if (((LiteralExpr *)expr->condition)->boolean == true)
				return expr->trueBranch;
			else
				return expr->falseBranch;
		}

		return expr;
	}
	Expr *Optimizer::OptLiteralExpr(LiteralExpr *expr)
	{
		return expr;
	}

	Expr *Optimizer::OptPrefixExpr(PrefixExpr *expr)
	{
		expr->right = OptExpr(expr->right);
		return OptFlow(expr);
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
		if (expr->expr->type == AST_LITERAL && ((LiteralExpr *)expr->expr)->literalType == LiteralExpr::Type::INTEGER)
		{
			auto intExpr = new LiteralExpr(expr->tagToken, Factorial(((LiteralExpr *)expr->expr)->iValue));
			SAFE_DELETE(expr);
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
			if (infix->left->type == AST_LITERAL && infix->right->type == AST_LITERAL)
			{
				Expr *newExpr = infix;
				auto tagToken = infix->tagToken;
				bool needToDelete = true;

				auto leftLiteral = ((LiteralExpr *)infix->left);
				auto rightLiteral = ((LiteralExpr *)infix->right);

				if (leftLiteral->literalType == LiteralExpr::Type::FLOATING && rightLiteral->literalType == LiteralExpr::Type::FLOATING)
				{
#define BIN_EXPR(x)       \
	if (infix->op == L#x) \
	newExpr = new LiteralExpr(tagToken, leftLiteral->dValue x rightLiteral->dValue)

					BIN_EXPR(+);
					else BIN_EXPR(-);
					else BIN_EXPR(*);
					else BIN_EXPR(/);
					else BIN_EXPR(==);
					else BIN_EXPR(!=);
					else BIN_EXPR(>);
					else BIN_EXPR(>=);
					else BIN_EXPR(<);
					else BIN_EXPR(<=);
					else needToDelete = false;

#undef BIN_EXPR

					if (needToDelete)
						SAFE_DELETE(infix);
					return newExpr;
				}
				else if (leftLiteral->literalType == LiteralExpr::Type::INTEGER && rightLiteral->literalType == LiteralExpr::Type::INTEGER)
				{
#define BIN_EXPR(x)       \
	if (infix->op == L#x) \
	newExpr = new LiteralExpr(tagToken, leftLiteral->iValue x rightLiteral->iValue)

					BIN_EXPR(+);
					else BIN_EXPR(-);
					else BIN_EXPR(*);
					else BIN_EXPR(/);
					else BIN_EXPR(%);
					else BIN_EXPR(&);
					else BIN_EXPR(|);
					else BIN_EXPR(^);
					else BIN_EXPR(==);
					else BIN_EXPR(!=);
					else BIN_EXPR(>);
					else BIN_EXPR(>=);
					else BIN_EXPR(<);
					else BIN_EXPR(<=);
					else BIN_EXPR(<<);
					else BIN_EXPR(>>);
					else needToDelete = false;

#undef BIN_EXPR

					if (needToDelete)
						SAFE_DELETE(infix);
					return newExpr;
				}
				else if (leftLiteral->literalType == LiteralExpr::Type::INTEGER && rightLiteral->literalType == LiteralExpr::Type::FLOATING)
				{
#define BIN_EXPR(x)       \
	if (infix->op == L#x) \
	newExpr = new LiteralExpr(tagToken, leftLiteral->iValue x rightLiteral->dValue)

					BIN_EXPR(+);
					else BIN_EXPR(-);
					else BIN_EXPR(*);
					else BIN_EXPR(/);
					else BIN_EXPR(==);
					else BIN_EXPR(!=);
					else BIN_EXPR(>);
					else BIN_EXPR(>=);
					else BIN_EXPR(<);
					else BIN_EXPR(<=);
					else needToDelete = false;

#undef BIN_EXPR

					if (needToDelete)
						SAFE_DELETE(infix);
					return newExpr;
				}
				else if (leftLiteral->literalType == LiteralExpr::Type::FLOATING && rightLiteral->literalType == LiteralExpr::Type::INTEGER)
				{

#define BIN_EXPR(x)       \
	if (infix->op == L#x) \
	newExpr = new LiteralExpr(tagToken, leftLiteral->dValue x rightLiteral->iValue)

					BIN_EXPR(+);
					else BIN_EXPR(-);
					else BIN_EXPR(*);
					else BIN_EXPR(/);
					else BIN_EXPR(==);
					else BIN_EXPR(!=);
					else BIN_EXPR(>);
					else BIN_EXPR(>=);
					else BIN_EXPR(<);
					else BIN_EXPR(<=);
					else needToDelete = false;

#undef BIN_EXPR

					if (needToDelete)
						SAFE_DELETE(infix);
					return newExpr;
				}
				else if (leftLiteral->literalType == LiteralExpr::Type::STRING && rightLiteral->literalType == LiteralExpr::Type::STRING)
				{
					auto strExpr = new LiteralExpr(infix->tagToken, leftLiteral->str + rightLiteral->str);
					SAFE_DELETE(infix);
					return strExpr;
				}
			}
		}
		else if (expr->type == AST_PREFIX)
		{
			auto prefix = (PrefixExpr *)expr;

			if (prefix->right->type == AST_LITERAL)
			{
				auto rightLiteralExpr = ((LiteralExpr *)prefix->right);
				if (rightLiteralExpr->literalType == LiteralExpr::Type::FLOATING && prefix->op == L"-")
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, -rightLiteralExpr->dValue);
					SAFE_DELETE(prefix);
					return numExpr;
				}
				else if (rightLiteralExpr->literalType == LiteralExpr::Type::INTEGER && prefix->op == L"-")
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, -rightLiteralExpr->iValue);
					SAFE_DELETE(prefix);
					return numExpr;
				}
				else if (rightLiteralExpr->literalType == LiteralExpr::Type::BOOLEAN && prefix->op == L"!")
				{
					auto boolExpr = new LiteralExpr(prefix->tagToken, !rightLiteralExpr->boolean);
					SAFE_DELETE(prefix);
					return boolExpr;
				}
				else if (rightLiteralExpr->literalType == LiteralExpr::Type::INTEGER && prefix->op == L"~")
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, ~rightLiteralExpr->iValue);
					SAFE_DELETE(prefix);
					return numExpr;
				}
			}
		}
		else if (expr->type == AST_POSTFIX)
		{
			auto postfix = (PostfixExpr *)expr;
			if (postfix->left->type == AST_LITERAL)
			{
				auto leftLiteralExpr = (LiteralExpr *)postfix->left;
				if (postfix->op == L"!")
				{
					auto numExpr = new LiteralExpr(postfix->tagToken, Factorial(leftLiteralExpr->iValue));
					SAFE_DELETE(postfix);
					return numExpr;
				}
			}
		}

		return expr;
	}
}