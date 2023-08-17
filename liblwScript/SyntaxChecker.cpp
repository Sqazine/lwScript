#include "SyntaxChecker.h"
#include "Utils.h"
namespace lws
{
    SyntaxChecker::SyntaxChecker()
        : mLoopDepth(0), mVarArgScopeDepth(false)
    {
    }
    SyntaxChecker::~SyntaxChecker()
    {
    }

    Stmt *SyntaxChecker::Check(Stmt *stmt)
    {
        return CheckStmt(stmt);
    }

    Stmt *SyntaxChecker::CheckStmt(Stmt *stmt)
    {
        switch (stmt->type)
        {
        case AST_ASTSTMTS:
            return CheckAstStmts((AstStmts *)stmt);
        case AST_RETURN:
            return CheckReturnStmt((ReturnStmt *)stmt);
        case AST_EXPR:
            return CheckExprStmt((ExprStmt *)stmt);
        case AST_VAR:
            return CheckVarStmt((VarStmt *)stmt);
        case AST_SCOPE:
            return CheckScopeStmt((ScopeStmt *)stmt);
        case AST_IF:
            return CheckIfStmt((IfStmt *)stmt);
        case AST_WHILE:
            return CheckWhileStmt((WhileStmt *)stmt);
        case AST_FUNCTION:
            return CheckFunctionStmt((FunctionStmt *)stmt);
        case AST_CLASS:
            return CheckClassStmt((ClassStmt *)stmt);
        default:
            return stmt;
        }
    }

    Stmt *SyntaxChecker::CheckAstStmts(AstStmts *stmt)
    {
        for (auto &s : stmt->stmts)
            s = CheckStmt(s);
        return stmt;
    }

    Stmt *SyntaxChecker::CheckVarStmt(VarStmt *stmt)
    {
        for (auto &[k, v] : stmt->variables)
        {
            if (k->type == AST_ARRAY)
            {
                auto arrayK = (ArrayExpr *)k;
                for (int32_t i = 0; i < arrayK->elements.size(); ++i)
                {
                    auto e = arrayK->elements[i];
                    e = CheckExpr(e);
                    if (e->type != AST_VAR_DESC && e->type != AST_VAR_ARG)
                        ERROR("only variable description or variable argument is available in destructing assign expr.");

                    if (e->type == AST_VAR_ARG && i != arrayK->elements.size() - 1)
                        ERROR("variable argument expr only available at last of destructing assignment expr.")
                }
            }
            else if (k->type == AST_VAR_DESC)
                k = CheckIdentifierExpr((IdentifierExpr *)k);
            else
                ERROR("only destructing assign expr or variable description is available in let/const stmt's binding part.");
        
            v=CheckExpr(v);
        }
        return stmt;
    }

    Stmt *SyntaxChecker::CheckExprStmt(ExprStmt *stmt)
    {
        stmt->expr = CheckExpr(stmt->expr);
        return stmt;
    }
    Stmt *SyntaxChecker::CheckIfStmt(IfStmt *stmt)
    {
        return CheckStmt(stmt);
    }
    Stmt *SyntaxChecker::CheckScopeStmt(ScopeStmt *stmt)
    {
        return CheckStmt(stmt);
    }
    Stmt *SyntaxChecker::CheckWhileStmt(WhileStmt *stmt)
    {
        return CheckStmt(stmt);
    }
    Stmt *SyntaxChecker::CheckEnumStmt(EnumStmt *stmt)
    {
        return CheckStmt(stmt);
    }
    Stmt *SyntaxChecker::CheckFunctionStmt(FunctionStmt *stmt)
    {
        return CheckStmt(stmt);
    }
    Stmt *SyntaxChecker::CheckClassStmt(ClassStmt *stmt)
    {
        return CheckStmt(stmt);
    }
    Stmt *SyntaxChecker::CheckReturnStmt(ReturnStmt *stmt)
    {
        return CheckStmt(stmt);
    }
    Stmt *SyntaxChecker::CheckBreakStmt(BreakStmt *stmt)
    {
        return stmt;
    }
    Stmt *SyntaxChecker::CheckContinueStmt(ContinueStmt *stmt)
    {
        return stmt;
    }
    Expr *SyntaxChecker::CheckExpr(Expr *expr)
    {
        switch (expr->type)
        {
        case AST_INT:
            return CheckIntNumExpr((IntNumExpr *)expr);
        case AST_REAL:
            return CheckRealNumExpr((RealNumExpr *)expr);
        case AST_STR:
            return CheckStrExpr((StrExpr *)expr);
        case AST_NULL:
            return CheckNullExpr((NullExpr *)expr);
        case AST_BOOL:
            return CheckBoolExpr((BoolExpr *)expr);
        case AST_IDENTIFIER:
            return CheckIdentifierExpr((IdentifierExpr *)expr);
        case AST_VAR_DESC:
            return CheckVarArgExpr((VarArgExpr *)expr);
        case AST_GROUP:
            return CheckGroupExpr((GroupExpr *)expr);
        case AST_ARRAY:
            return CheckArrayExpr((ArrayExpr *)expr);
        case AST_DICT:
            return CheckDictExpr((DictExpr *)expr);
        case AST_PREFIX:
            return CheckPrefixExpr((PrefixExpr *)expr);
        case AST_INFIX:
            return CheckInfixExpr((InfixExpr *)expr);
        case AST_POSTFIX:
            return CheckPostfixExpr((PostfixExpr *)expr);
        case AST_CONDITION:
            return CheckConditionExpr((ConditionExpr *)expr);
        case AST_INDEX:
            return CheckIndexExpr((IndexExpr *)expr);
        case AST_REF:
            return CheckRefExpr((RefExpr *)expr);
        case AST_LAMBDA:
            return CheckLambdaExpr((LambdaExpr *)expr);
        case AST_DOT:
            return CheckDotExpr((DotExpr *)expr);
        case AST_CALL:
            return CheckCallExpr((CallExpr *)expr);
        case AST_NEW:
            return CheckNewExpr((NewExpr *)expr);
        case AST_THIS:
            return CheckThisExpr((ThisExpr *)expr);
        case AST_BASE:
            return CheckBaseExpr((BaseExpr *)expr);
        case AST_BLOCK:
            return CheckBlockExpr((BlockExpr *)expr);
        case AST_ANONY_OBJ:
            return CheckAnonymousObjExpr((AnonyObjExpr *)expr);
        case AST_VAR_ARG:
            return CheckVarArgExpr((VarArgExpr *)expr);
        case AST_FACTORIAL:
            return CheckFactorialExpr((FactorialExpr *)expr);
        case AST_APPREGATE:
            return CheckAppregateExpr((AppregateExpr *)expr);
        default:
            return expr;
        }
    }
    Expr *SyntaxChecker::CheckInfixExpr(InfixExpr *expr)
    {
        expr->left = CheckExpr(expr->left);
        expr->right = CheckExpr(expr->right);
        return expr;
    }
    Expr *SyntaxChecker::CheckIntNumExpr(IntNumExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckRealNumExpr(RealNumExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckBoolExpr(BoolExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckPrefixExpr(PrefixExpr *expr)
    {
        expr->right = CheckExpr(expr->right);
        return expr;
    }
    Expr *SyntaxChecker::CheckPostfixExpr(PostfixExpr *expr)
    {
        expr->left = CheckExpr(expr->left);
        return expr;
    }
    Expr *SyntaxChecker::CheckConditionExpr(ConditionExpr *expr)
    {
        expr->condition = CheckExpr(expr->condition);
        expr->trueBranch = CheckExpr(expr->trueBranch);
        expr->falseBranch = CheckExpr(expr->falseBranch);
        return expr;
    }
    Expr *SyntaxChecker::CheckStrExpr(StrExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckNullExpr(NullExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckGroupExpr(GroupExpr *expr)
    {
        expr->expr = CheckExpr(expr->expr);
        return expr;
    }
    Expr *SyntaxChecker::CheckArrayExpr(ArrayExpr *expr)
    {
        for (auto &e : expr->elements)
            e = CheckExpr(e);
        return expr;
    }

    Expr *SyntaxChecker::CheckAppregateExpr(AppregateExpr *expr)
    {
        return expr;
    }

    Expr *SyntaxChecker::CheckDictExpr(DictExpr *expr)
    {
        for (auto &[k, v] : expr->elements)
        {
            if (!IsConstantLiteral(k))
                ERROR("Dict keys can only have constant literals (int num,real num,string,boolean,null or variable identifier).");
            v = CheckExpr(v);
        }
        return expr;
    }
    Expr *SyntaxChecker::CheckIndexExpr(IndexExpr *expr)
    {
        expr->ds = CheckExpr(expr->ds);
        expr->index = CheckExpr(expr->index);
        return expr;
    }
    Expr *SyntaxChecker::CheckNewExpr(NewExpr *expr)
    {
        if (expr->callee->type != AST_CALL && expr->callee->type != AST_ANONY_OBJ)
            ERROR(L"Not a valid new expr,call expr or anonymous object expr is necessary followed 'new' keyword.");

        return expr;
    }
    Expr *SyntaxChecker::CheckThisExpr(ThisExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckBaseExpr(BaseExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckIdentifierExpr(IdentifierExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckLambdaExpr(LambdaExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckBlockExpr(BlockExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckCallExpr(CallExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckDotExpr(DotExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckRefExpr(RefExpr *expr)
    {
        if(expr->refExpr->type!=AST_IDENTIFIER)
            ERROR("Only left value is available for reference.");
        return expr;
    }
    Expr *SyntaxChecker::CheckAnonymousObjExpr(AnonyObjExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckVarArgExpr(VarArgExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxChecker::CheckFactorialExpr(FactorialExpr *expr)
    {
        return expr;
    }

    bool SyntaxChecker::IsConstantLiteral(Expr *expr)
    {
        switch (expr->type)
        {
        case AST_INT:
        case AST_REAL:
        case AST_STR:
        case AST_BOOL:
        case AST_NULL:
        case AST_IDENTIFIER:
            return true;
        default:
            return false;
        }
        return false;
    }
}