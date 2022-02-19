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

    std::string IntNumExpr::Stringify()
    {
        return std::to_string(value);
    }
    AstType IntNumExpr::Type()
    {
        return AstType::INT;
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
    std::string RealNumExpr::Stringify()
    {
        return std::to_string(value);
    }
    AstType RealNumExpr::Type()
    {
        return AstType::REAL;
    }

    StrExpr::StrExpr()
    {
    }
    StrExpr::StrExpr(std::string_view str)
        : value(str)
    {
    }

    StrExpr::~StrExpr()
    {
    }

    std::string StrExpr::Stringify()
    {
        return "\"" + value + "\"";
    }

    AstType StrExpr::Type()
    {
        return AstType::STR;
    }

    NilExpr::NilExpr()
    {
    }
    NilExpr::~NilExpr()
    {
    }

    std::string NilExpr::Stringify()
    {
        return "nil";
    }
    AstType NilExpr::Type()
    {
        return AstType::NIL;
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

    std::string BoolExpr::Stringify()
    {
        return value ? "true" : "false";
    }
    AstType BoolExpr::Type()
    {
        return AstType::BOOL;
    }

    IdentifierExpr::IdentifierExpr()
    {
    }
    IdentifierExpr::IdentifierExpr(std::string_view literal)
        : literal(literal)
    {
    }
    IdentifierExpr::~IdentifierExpr()
    {
    }

    std::string IdentifierExpr::Stringify()
    {
        return literal;
    }
    AstType IdentifierExpr::Type()
    {
        return AstType::IDENTIFIER;
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

    std::string ArrayExpr::Stringify()
    {
        std::string result = "[";

        if (!elements.empty())
        {
            for (auto e : elements)
                result += e->Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += "]";
        return result;
    }
    AstType ArrayExpr::Type()
    {
        return AstType::ARRAY;
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

    std::string TableExpr::Stringify()
    {
        std::string result = "{";

        if (!elements.empty())
        {
            for (auto [key, value] : elements)
                result += key->Stringify() + ":" + value->Stringify();
            result = result.substr(0, result.size() - 1);
        }
        result += "}";
        return result;
    }
    AstType TableExpr::Type()
    {
        return AstType::TABLE;
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
    std::string GroupExpr::Stringify()
    {
        return "(" + expr->Stringify() + ")";
    }
    AstType GroupExpr::Type()
    {
        return AstType::GROUP;
    }

    PrefixExpr::PrefixExpr()
        : right(nullptr)
    {
    }
    PrefixExpr::PrefixExpr(std::string_view op, Expr *right)
        : op(op), right(right)
    {
    }
    PrefixExpr::~PrefixExpr()
    {
        delete right;
        right = nullptr;
    }

    std::string PrefixExpr::Stringify()
    {
        return op + right->Stringify();
    }
    AstType PrefixExpr::Type()
    {
        return AstType::PREFIX;
    }

    InfixExpr::InfixExpr()
        : left(nullptr), right(nullptr)
    {
    }
    InfixExpr::InfixExpr(std::string_view op, Expr *left, Expr *right)
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

    std::string InfixExpr::Stringify()
    {
        return left->Stringify() + op + right->Stringify();
    }
    AstType InfixExpr::Type()
    {
        return AstType::INFIX;
    }

    ConditionExpr::ConditionExpr()
        : condition(nullptr), trueBranch(nullptr), falseBranch(nullptr)
    {
    }
    ConditionExpr::ConditionExpr(Expr *condition, Expr *left, Expr *right)
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

    std::string ConditionExpr::Stringify()
    {
        return condition->Stringify() + "?" + trueBranch->Stringify() + ":" + falseBranch->Stringify();
    }
    AstType ConditionExpr::Type()
    {
        return AstType::CONDITION;
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
    std::string IndexExpr::Stringify()
    {
        return ds->Stringify() + "[" + index->Stringify() + "]";
    }

    AstType IndexExpr::Type()
    {
        return AstType::INDEX;
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

    std::string RefExpr::Stringify()
    {
        return "&" + refExpr->Stringify();
    }

    AstType RefExpr::Type()
    {
        return AstType::REF;
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

    std::string LambdaExpr::Stringify()
    {
        std::string result = "fn(";
        if (!parameters.empty())
        {
            for (auto param : parameters)
                result += param->Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += ")";
        result += body->Stringify();
        return result;
    }
    AstType LambdaExpr::Type()
    {
        return AstType::LAMBDA;
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
    std::string FunctionCallExpr::Stringify()
    {
        std::string result = name->Stringify() + "(";

        if (!arguments.empty())
        {
            for (const auto &arg : arguments)
                result += arg->Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += ")";
        return result;
    }
    AstType FunctionCallExpr::Type()
    {
        return AstType::FUNCTION_CALL;
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

    std::string FieldCallExpr::Stringify()
    {
        return callee->Stringify() + "." + callMember->Stringify();
    }
    AstType FieldCallExpr::Type()
    {
        return AstType::FIELD_CALL;
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

    std::string ExprStmt::Stringify()
    {
        return expr->Stringify() + ";";
    }
    AstType ExprStmt::Type()
    {
        return AstType::EXPR;
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

    std::string LetStmt::Stringify()
    {
        std::string result = "let ";
        if (!variables.empty())
        {
            for (auto [key, value] : variables)
                result += key->Stringify() + "=" + value->Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        return result + ";";
    }

    AstType LetStmt::Type()
    {
        return AstType::LET;
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

    std::string ReturnStmt::Stringify()
    {
        return "return " + expr->Stringify() + ";";
    }
    AstType ReturnStmt::Type()
    {
        return AstType::RETURN;
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

    std::string IfStmt::Stringify()
    {
        std::string result;
        result = "if(" + condition->Stringify() + ")" + thenBranch->Stringify();
        if (elseBranch != nullptr)
            result += "else " + elseBranch->Stringify();
        return result;
    }
    AstType IfStmt::Type()
    {
        return AstType::IF;
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

    std::string ScopeStmt::Stringify()
    {
        std::string result = "{";
        for (const auto &stmt : stmts)
            result += stmt->Stringify();
        result += "}";
        return result;
    }

    AstType ScopeStmt::Type()
    {
        return AstType::SCOPE;
    }

    WhileStmt::WhileStmt()
        : condition(nullptr), body(nullptr)
    {
    }
    WhileStmt::WhileStmt(Expr *condition, Stmt *body)
        : condition(condition), body(body)
    {
    }
    WhileStmt::~WhileStmt()
    {
        delete condition;
        condition = nullptr;
        delete body;
        body = nullptr;
    }

    std::string WhileStmt::Stringify()
    {
        return "while(" + condition->Stringify() + ")" + body->Stringify();
    }
    AstType WhileStmt::Type()
    {
        return AstType::WHILE;
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

    std::string FunctionStmt::Stringify()
    {
        std::string result = "fn " + name->Stringify() + "(";
        if (!parameters.empty())
        {
            for (auto param : parameters)
                result += param->Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += ")";
        result += body->Stringify();
        return result;
    }
    AstType FunctionStmt::Type()
    {
        return AstType::FUNCTION;
    }

    FieldStmt::FieldStmt()
    {
    }
    FieldStmt::FieldStmt(std::string name,
                         std::vector<LetStmt *> letStmts,
                         std::vector<FunctionStmt *> fnStmts,
                         std::vector<IdentifierExpr *> containedFields)
        : name(name), letStmts(letStmts), fnStmts(fnStmts), containedFields(containedFields)
    {
    }
    FieldStmt::~FieldStmt()
    {
        std::vector<IdentifierExpr *>().swap(containedFields);
        std::vector<LetStmt *>().swap(letStmts);
        std::vector<FunctionStmt *>().swap(fnStmts);
    }

    std::string FieldStmt::Stringify()
    {
        std::string result = "field " + name;
        if (!containedFields.empty())
        {
            result += ":";
            for (const auto &containedField : containedFields)
                result += containedField->Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += "{\n";
        for (auto letStmt : letStmts)
            result += letStmt->Stringify() + "\n";
        for (auto fnStmt : fnStmts)
            result += fnStmt->Stringify() + "\n";
        return result + "}";
    }
    AstType FieldStmt::Type()
    {
        return AstType::FIELD;
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

    std::string AstStmts::Stringify()
    {
        std::string result;
        for (const auto &stmt : stmts)
            result += stmt->Stringify();
        return result;
    }
    AstType AstStmts::Type()
    {
        return AstType::ASTSTMTS;
    }
}
