#include "SyntaxChecker.h"
#include "Utils.h"
#include "Logger.h"

namespace lwscript
{
    SyntaxChecker::SyntaxChecker()
        : mLoopDepth(0), mVarArgScopeDepth(false), mClassScopeDepth(0)
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
        switch (stmt->kind)
        {
        case AstKind::ASTSTMTS:
            return CheckAstStmts((AstStmts *)stmt);
        case AstKind::RETURN:
            return CheckReturnStmt((ReturnStmt *)stmt);
        case AstKind::EXPR:
            return CheckExprStmt((ExprStmt *)stmt);
        case AstKind::VAR:
            return CheckVarStmt((VarStmt *)stmt);
        case AstKind::SCOPE:
            return CheckScopeStmt((ScopeStmt *)stmt);
        case AstKind::IF:
            return CheckIfStmt((IfStmt *)stmt);
        case AstKind::WHILE:
            return CheckWhileStmt((WhileStmt *)stmt);
        case AstKind::FUNCTION:
            return CheckFunctionStmt((FunctionStmt *)stmt);
        case AstKind::CLASS:
            return CheckClassStmt((ClassStmt *)stmt);
        case AstKind::MODULE:
            return CheckModuleStmt((ModuleStmt *)stmt);
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
            if (k->kind == AstKind::ARRAY)
            {
                auto arrayK = (ArrayExpr *)k;
                for (int32_t i = 0; i < arrayK->elements.size(); ++i)
                {
                    auto e = arrayK->elements[i];
                    e = CheckExpr(e);
                    if (e->kind != AstKind::VAR_DESC && e->kind != AstKind::VAR_ARG)
                        Logger::Error(e->tagToken, L"only variable description or variable argument is available in destructing assign expr.");

                    if (e->kind == AstKind::VAR_ARG && i != arrayK->elements.size() - 1)
                        Logger::Error(k->tagToken, L"variable argument expr only available at the end of destructing assignment expr.");
                }
            }
            else if (k->kind == AstKind::VAR_DESC)
                k = CheckExpr(k);
            else
                Logger::Error(k->tagToken, L"only destructing assign expr or variable description is available in let/const stmt's binding part.");

            v = CheckExpr(v);
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
        return stmt;
    }
    Stmt *SyntaxChecker::CheckScopeStmt(ScopeStmt *stmt)
    {
        for (auto &s : stmt->stmts)
            s = CheckStmt(s);
        return stmt;
    }
    Stmt *SyntaxChecker::CheckWhileStmt(WhileStmt *stmt)
    {
        stmt->condition = CheckExpr(stmt->condition);
        stmt->body = (ScopeStmt *)CheckScopeStmt(stmt->body);
        if (stmt->increment)
            stmt->increment = (ScopeStmt *)CheckScopeStmt(stmt->increment);
        return stmt;
    }
    Stmt *SyntaxChecker::CheckEnumStmt(EnumStmt *stmt)
    {
        stmt->name = (IdentifierExpr *)CheckIdentifierExpr(stmt->name);
        for (auto &[k, v] : stmt->enumItems)
            v = CheckExpr(v);
        return stmt;
    }
    Stmt *SyntaxChecker::CheckFunctionStmt(FunctionStmt *stmt)
    {
        for (int32_t i = 0; i < stmt->parameters.size(); ++i)
        {
            auto e = stmt->parameters[i];
            e = (VarDescExpr *)CheckVarDescExpr(e);

            if (e->name->kind == AstKind::VAR_ARG && i != stmt->parameters.size() - 1)
                Logger::Error(e->tagToken, L"variable argument expr only available at the end of function parameter list.");
        }

        stmt->body = (ScopeStmt *)CheckScopeStmt(stmt->body);
        return stmt;
    }
    Stmt *SyntaxChecker::CheckClassStmt(ClassStmt *stmt)
    {
        return stmt;
    }
    Stmt *SyntaxChecker::CheckReturnStmt(ReturnStmt *stmt)
    {
        if (stmt->expr)
            stmt->expr = CheckExpr(stmt->expr);
        return stmt;
    }
    Stmt *SyntaxChecker::CheckBreakStmt(BreakStmt *stmt)
    {
        return stmt;
    }
    Stmt *SyntaxChecker::CheckContinueStmt(ContinueStmt *stmt)
    {
        return stmt;
    }

    Stmt *SyntaxChecker::CheckModuleStmt(ModuleStmt *stmt)
    {
        return stmt;
    }

    Expr *SyntaxChecker::CheckExpr(Expr *expr)
    {
        switch (expr->kind)
        {
        case AstKind::LITERAL:
            return CheckLiteralExpr((LiteralExpr *)expr);
        case AstKind::IDENTIFIER:
            return CheckIdentifierExpr((IdentifierExpr *)expr);
        case AstKind::VAR_DESC:
            return CheckVarArgExpr((VarArgExpr *)expr);
        case AstKind::GROUP:
            return CheckGroupExpr((GroupExpr *)expr);
        case AstKind::ARRAY:
            return CheckArrayExpr((ArrayExpr *)expr);
        case AstKind::DICT:
            return CheckDictExpr((DictExpr *)expr);
        case AstKind::PREFIX:
            return CheckPrefixExpr((PrefixExpr *)expr);
        case AstKind::INFIX:
            return CheckInfixExpr((InfixExpr *)expr);
        case AstKind::POSTFIX:
            return CheckPostfixExpr((PostfixExpr *)expr);
        case AstKind::CONDITION:
            return CheckConditionExpr((ConditionExpr *)expr);
        case AstKind::INDEX:
            return CheckIndexExpr((IndexExpr *)expr);
        case AstKind::REF:
            return CheckRefExpr((RefExpr *)expr);
        case AstKind::LAMBDA:
            return CheckLambdaExpr((LambdaExpr *)expr);
        case AstKind::DOT:
            return CheckDotExpr((DotExpr *)expr);
        case AstKind::CALL:
            return CheckCallExpr((CallExpr *)expr);
        case AstKind::NEW:
            return CheckNewExpr((NewExpr *)expr);
        case AstKind::THIS:
            return CheckThisExpr((ThisExpr *)expr);
        case AstKind::BASE:
            return CheckBaseExpr((BaseExpr *)expr);
        case AstKind::COMPOUND:
            return CheckCompoundExpr((CompoundExpr *)expr);
        case AstKind::STRUCT:
            return CheckStructExpr((StructExpr *)expr);
        case AstKind::VAR_ARG:
            return CheckVarArgExpr((VarArgExpr *)expr);
        case AstKind::FACTORIAL:
            return CheckFactorialExpr((FactorialExpr *)expr);
        case AstKind::APPREGATE:
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
    Expr *SyntaxChecker::CheckLiteralExpr(LiteralExpr *expr)
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
                Logger::Error(k->tagToken, L"Dict keys can only have constant literals (int num,real num,string,boolean,null or variable identifier).");
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
        if (expr->callee->kind != AstKind::CALL)
            Logger::Error(expr->callee->tagToken, L"Not a valid new expr,call expr is necessary followed 'new' keyword.");

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
        for (int32_t i = 0; i < expr->parameters.size(); ++i)
        {
            auto e = expr->parameters[i];
            e = (VarDescExpr *)CheckVarDescExpr(e);

            if (e->name->kind == AstKind::VAR_ARG && i != expr->parameters.size() - 1)
                Logger::Error(e->tagToken, L"variable argument expr only available at the end of lambda parameter list.");
        }

        expr->body = (ScopeStmt *)CheckScopeStmt(expr->body);
        return expr;
    }
    Expr *SyntaxChecker::CheckCompoundExpr(CompoundExpr *expr)
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
        if (expr->refExpr->kind != AstKind::IDENTIFIER && expr->refExpr->kind != AstKind::INDEX)
            Logger::Error(expr->tagToken, L"Only left value is available for reference.");
        return expr;
    }
    Expr *SyntaxChecker::CheckStructExpr(StructExpr *expr)
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

    Expr *SyntaxChecker::CheckVarDescExpr(VarDescExpr *expr)
    {
        if (expr->name->kind != AstKind::IDENTIFIER && expr->name->kind != AstKind::VAR_ARG)
            Logger::Error(expr->tagToken, L"Only identifier or variable argument is available at variable declaration or param declaration");
        return expr;
    }

    bool SyntaxChecker::IsConstantLiteral(Expr *expr)
    {
        switch (expr->kind)
        {
        case AstKind::LITERAL:
        case AstKind::IDENTIFIER:
            return true;
        default:
            return false;
        }
        return false;
    }
}