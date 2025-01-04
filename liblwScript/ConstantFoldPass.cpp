#include "ConstantFoldPass.h"
#include "Utils.h"

namespace lwscript
{
#ifdef CONSTANT_FOLD_OPT
	Stmt *ConstantFoldPass::ExecuteAstStmts(AstStmts *stmt)
	{
		for (auto &s : stmt->stmts)
			s = ExecuteStmt(s);
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteExprStmt(ExprStmt *stmt)
	{
		stmt->expr = ExecuteExpr(stmt->expr);
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteIfStmt(IfStmt *stmt)
	{
		stmt->condition = ExecuteExpr(stmt->condition);
		stmt->thenBranch = ExecuteStmt(stmt->thenBranch);
		if (stmt->elseBranch)
			stmt->elseBranch = ExecuteStmt(stmt->elseBranch);

		if (stmt->condition->kind == AstKind::LITERAL && ((LiteralExpr *)stmt->condition)->literalType == LiteralExpr::Type::BOOLEAN)
		{
			if (((LiteralExpr *)stmt->condition)->boolean == true)
				return stmt->thenBranch;
			else
				return stmt->elseBranch;
		}

		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteScopeStmt(ScopeStmt *stmt)
	{
		for (auto &s : stmt->stmts)
			s = ExecuteStmt(s);
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteWhileStmt(WhileStmt *stmt)
	{
		stmt->condition = ExecuteExpr(stmt->condition);
		stmt->body = (ScopeStmt *)ExecuteScopeStmt(stmt->body);
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteEnumStmt(EnumStmt *stmt)
	{
		for (auto &[k, v] : stmt->enumItems)
			v = ExecuteExpr(v);
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteReturnStmt(ReturnStmt *stmt)
	{
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteVarStmt(VarStmt *stmt)
	{
		for (auto &[k, v] : stmt->variables)
			v = ExecuteExpr(v);
		return stmt;
	}

	Stmt *ConstantFoldPass::ExecuteFunctionStmt(FunctionStmt *stmt)
	{
		for (auto &e : stmt->parameters)
			e = (VarDescExpr *)ExecuteVarDescExpr(e);

		stmt->body = (ScopeStmt *)ExecuteScopeStmt(stmt->body);

		return stmt;
	}

	Stmt *ConstantFoldPass::ExecuteClassStmt(ClassStmt *stmt)
	{
		for (auto &varStmt : stmt->varItems)
			varStmt = (VarStmt *)ExecuteVarStmt(varStmt);

		for (auto &fnStmt : stmt->fnItems)
			fnStmt = (FunctionStmt *)ExecuteFunctionStmt(fnStmt);

		return stmt;
	}

	Stmt *ConstantFoldPass::ExecuteBreakStmt(BreakStmt *stmt)
	{
		return stmt;
	}

	Stmt *ConstantFoldPass::ExecuteContinueStmt(ContinueStmt *stmt)
	{
		return stmt;
	}

	Stmt *ConstantFoldPass::ExecuteModuleStmt(ModuleStmt *stmt)
	{
		return stmt;
	}

	Expr *ConstantFoldPass::ExecuteInfixExpr(InfixExpr *expr)
	{
		expr->left = ExecuteExpr(expr->left);
		expr->right = ExecuteExpr(expr->right);

		return ConstantFold(expr);
	}
	Expr *ConstantFoldPass::ExecutePostfixExpr(PostfixExpr *expr)
	{
		expr->left = ExecuteExpr(expr->left);
		return ConstantFold(expr);
	}
	Expr *ConstantFoldPass::ExecuteConditionExpr(ConditionExpr *expr)
	{
		expr->condition = ExecuteExpr(expr->condition);
		expr->trueBranch = ExecuteExpr(expr->trueBranch);
		expr->falseBranch = ExecuteExpr(expr->falseBranch);

		if (expr->condition->kind == AstKind::LITERAL && ((LiteralExpr *)expr->condition)->literalType == LiteralExpr::Type::BOOLEAN)
		{
			if (((LiteralExpr *)expr->condition)->boolean == true)
				return expr->trueBranch;
			else
				return expr->falseBranch;
		}

		return expr;
	}
	Expr *ConstantFoldPass::ExecuteLiteralExpr(LiteralExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecutePrefixExpr(PrefixExpr *expr)
	{
		expr->right = ExecuteExpr(expr->right);
		return ConstantFold(expr);
	}

	Expr *ConstantFoldPass::ExecuteGroupExpr(GroupExpr *expr)
	{
		return ExecuteExpr(expr->expr);
	}
	Expr *ConstantFoldPass::ExecuteArrayExpr(ArrayExpr *expr)
	{
		for (auto &e : expr->elements)
			e = ExecuteExpr(e);
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteAppregateExpr(AppregateExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteDictExpr(DictExpr *expr)
	{
		for (auto &[k, v] : expr->elements)
		{
			k = ExecuteExpr(k);
			v = ExecuteExpr(v);
		}
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteIndexExpr(IndexExpr *expr)
	{
		expr->ds = ExecuteExpr(expr->ds);
		expr->index = ExecuteExpr(expr->index);
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteIdentifierExpr(IdentifierExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteLambdaExpr(LambdaExpr *expr)
	{
		for (auto &e : expr->parameters)
			e = (VarDescExpr *)ExecuteVarDescExpr(e);
		expr->body = (ScopeStmt *)ExecuteScopeStmt(expr->body);
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteCompoundExpr(CompoundExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteDotExpr(DotExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteCallExpr(CallExpr *expr)
	{
		expr->callee = ExecuteExpr(expr->callee);
		for (auto &arg : expr->arguments)
			arg = ExecuteExpr(arg);
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteNewExpr(NewExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteThisExpr(ThisExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteBaseExpr(BaseExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteFactorialExpr(FactorialExpr *expr)
	{
		if (expr->expr->kind == AstKind::LITERAL && ((LiteralExpr *)expr->expr)->literalType == LiteralExpr::Type::INTEGER)
		{
			auto intExpr = new LiteralExpr(expr->tagToken, Factorial(((LiteralExpr *)expr->expr)->iValue));
			SAFE_DELETE(expr);
			return intExpr;
		}
		else
		{
			expr->expr = ExecuteExpr(expr->expr);
			return expr;
		}
	}

	Expr *ConstantFoldPass::ExecuteVarDescExpr(VarDescExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteRefExpr(RefExpr *expr)
	{
		expr->refExpr = (IdentifierExpr *)ExecuteExpr(expr->refExpr);
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteStructExpr(StructExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteVarArgExpr(VarArgExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ConstantFold(Expr *expr)
	{
		if (expr->kind == AstKind::INFIX)
		{
			auto infix = (InfixExpr *)expr;
			if (infix->left->kind == AstKind::LITERAL && infix->right->kind == AstKind::LITERAL)
			{
				Expr *newExpr = infix;
				auto tagToken = infix->tagToken;
				bool needToDelete = true;

				auto leftLiteral = ((LiteralExpr *)infix->left);
				auto rightLiteral = ((LiteralExpr *)infix->right);

				if (leftLiteral->literalType == LiteralExpr::Type::FLOATING && rightLiteral->literalType == LiteralExpr::Type::FLOATING)
				{
#define BIN_EXPR(x)            \
	if (infix->op == TEXT(#x)) \
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
#define BIN_EXPR(x)            \
	if (infix->op == TEXT(#x)) \
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
#define BIN_EXPR(x)            \
	if (infix->op == TEXT(#x)) \
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

#define BIN_EXPR(x)            \
	if (infix->op == TEXT(#x)) \
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
		else if (expr->kind == AstKind::PREFIX)
		{
			auto prefix = (PrefixExpr *)expr;

			if (prefix->right->kind == AstKind::LITERAL)
			{
				auto rightLiteralExpr = ((LiteralExpr *)prefix->right);
				if (rightLiteralExpr->literalType == LiteralExpr::Type::FLOATING && prefix->op == TEXT("-"))
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, -rightLiteralExpr->dValue);
					SAFE_DELETE(prefix);
					return numExpr;
				}
				else if (rightLiteralExpr->literalType == LiteralExpr::Type::INTEGER && prefix->op == TEXT("-"))
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, -rightLiteralExpr->iValue);
					SAFE_DELETE(prefix);
					return numExpr;
				}
				else if (rightLiteralExpr->literalType == LiteralExpr::Type::BOOLEAN && prefix->op == TEXT("!"))
				{
					auto boolExpr = new LiteralExpr(prefix->tagToken, !rightLiteralExpr->boolean);
					SAFE_DELETE(prefix);
					return boolExpr;
				}
				else if (rightLiteralExpr->literalType == LiteralExpr::Type::INTEGER && prefix->op == TEXT("~"))
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, ~rightLiteralExpr->iValue);
					SAFE_DELETE(prefix);
					return numExpr;
				}
			}
		}
		else if (expr->kind == AstKind::POSTFIX)
		{
			auto postfix = (PostfixExpr *)expr;
			if (postfix->left->kind == AstKind::LITERAL)
			{
				auto leftLiteralExpr = (LiteralExpr *)postfix->left;
				if (postfix->op == TEXT("!"))
				{
					auto numExpr = new LiteralExpr(postfix->tagToken, Factorial(leftLiteralExpr->iValue));
					SAFE_DELETE(postfix);
					return numExpr;
				}
			}
		}

		return expr;
	}

#endif
}