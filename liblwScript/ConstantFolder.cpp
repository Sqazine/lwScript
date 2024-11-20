#include "ConstantFolder.h"
#include "Utils.h"

namespace lwscript
{
	Stmt *ConstantFolder::Fold(Stmt *stmt)
	{
		return FoldStmt(stmt);
	}

	Stmt *ConstantFolder::FoldStmt(Stmt *stmt)
	{
		switch (stmt->kind)
		{
		case AstKind::ASTSTMTS:
			return FoldAstStmts((AstStmts *)stmt);
		case AstKind::RETURN:
			return FoldReturnStmt((ReturnStmt *)stmt);
		case AstKind::EXPR:
			return FoldExprStmt((ExprStmt *)stmt);
		case AstKind::VAR:
			return FoldVarStmt((VarStmt *)stmt);
		case AstKind::SCOPE:
			return FoldScopeStmt((ScopeStmt *)stmt);
		case AstKind::IF:
			return FoldIfStmt((IfStmt *)stmt);
		case AstKind::WHILE:
			return FoldWhileStmt((WhileStmt *)stmt);
		case AstKind::FUNCTION:
			return FoldFunctionStmt((FunctionStmt *)stmt);
		case AstKind::CLASS:
			return FoldClassStmt((ClassStmt *)stmt);
		case AstKind::MODULE:
			return FoldModuleStmt((ModuleStmt *)stmt);
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

		if (stmt->condition->kind == AstKind::LITERAL && ((LiteralExpr *)stmt->condition)->literalType == LiteralExpr::Type::BOOLEAN)
		{
			if (((LiteralExpr *)stmt->condition)->boolean == true)
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
		return stmt;
	}
	Stmt *ConstantFolder::FoldVarStmt(VarStmt *stmt)
	{
		for (auto &[k, v] : stmt->variables)
			v = FoldExpr(v);
		return stmt;
	}

	Stmt *ConstantFolder::FoldFunctionStmt(FunctionStmt *stmt)
	{
		for (auto &e : stmt->parameters)
			e = (VarDescExpr *)FoldVarDescExpr(e);

		stmt->body = (ScopeStmt *)FoldScopeStmt(stmt->body);

		return stmt;
	}

	Stmt *ConstantFolder::FoldClassStmt(ClassStmt *stmt)
	{
		for (auto &varStmt : stmt->varItems)
			varStmt = (VarStmt *)FoldVarStmt(varStmt);

		for (auto &fnStmt : stmt->fnItems)
			fnStmt = (FunctionStmt *)FoldFunctionStmt(fnStmt);

		return stmt;
	}

	Stmt *ConstantFolder::FoldModuleStmt(ModuleStmt *stmt)
	{
		return stmt;
	}

	Expr *ConstantFolder::FoldExpr(Expr *expr)
	{
		switch (expr->kind)
		{
		case AstKind::LITERAL:
			return FoldLiteralExpr((LiteralExpr *)expr);
		case AstKind::IDENTIFIER:
			return FoldIdentifierExpr((IdentifierExpr *)expr);
		case AstKind::GROUP:
			return FoldGroupExpr((GroupExpr *)expr);
		case AstKind::ARRAY:
			return FoldArrayExpr((ArrayExpr *)expr);
		case AstKind::INDEX:
			return FoldIndexExpr((IndexExpr *)expr);
		case AstKind::PREFIX:
			return FoldPrefixExpr((PrefixExpr *)expr);
		case AstKind::INFIX:
			return FoldInfixExpr((InfixExpr *)expr);
		case AstKind::POSTFIX:
			return FoldPostfixExpr((PostfixExpr *)expr);
		case AstKind::CONDITION:
			return FoldConditionExpr((ConditionExpr *)expr);
		case AstKind::REF:
			return FoldRefExpr((RefExpr *)expr);
		case AstKind::CALL:
			return FoldCallExpr((CallExpr *)expr);
		case AstKind::DOT:
			return FoldDotExpr((DotExpr *)expr);
		case AstKind::LAMBDA:
			return FoldLambdaExpr((LambdaExpr *)expr);
		case AstKind::FACTORIAL:
			return FoldFactorialExpr((FactorialExpr *)expr);
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

		if (expr->condition->kind == AstKind::LITERAL && ((LiteralExpr *)expr->condition)->literalType == LiteralExpr::Type::BOOLEAN)
		{
			if (((LiteralExpr *)expr->condition)->boolean == true)
				return expr->trueBranch;
			else
				return expr->falseBranch;
		}

		return expr;
	}
	Expr *ConstantFolder::FoldLiteralExpr(LiteralExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFolder::FoldPrefixExpr(PrefixExpr *expr)
	{
		expr->right = FoldExpr(expr->right);
		return ConstantFold(expr);
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
	Expr *ConstantFolder::FoldDictExpr(DictExpr *expr)
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
			e = (VarDescExpr *)FoldVarDescExpr(e);
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

	Expr *ConstantFolder::FoldFactorialExpr(FactorialExpr *expr)
	{
		if (expr->expr->kind == AstKind::LITERAL && ((LiteralExpr *)expr->expr)->literalType == LiteralExpr::Type::INTEGER)
		{
			auto intExpr = new LiteralExpr(expr->tagToken, Factorial(((LiteralExpr *)expr->expr)->iValue));
			SAFE_DELETE(expr);
			return intExpr;
		}
		else
		{
			expr->expr = FoldExpr(expr->expr);
			return expr;
		}
	}

	Expr *ConstantFolder::FoldVarDescExpr(VarDescExpr *expr)
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
}