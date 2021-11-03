#pragma once
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Ast.h"
#include "Utils.h"
namespace lws
{
	enum class Precedence
	{
		LOWEST = 0,
		ASSIGN,		 // = += -= *= /= %= &= |= ^= <<= >>=
		CONDITION,	 // ?
		OR,			 // ||
		AND,		 // &&
		BIT_OR,		 // |
		BIT_XOR,	 // ^
		BIT_AND,	 // &
		EQUAL,		 // == !=
		COMPARE,	 // < <= > >=
		BIT_SHIFT,	 // >> <<
		ADD_PLUS,	 // + -
		MUL_DIV_MOD, // * / %
		UNARY,		 // ++ -- ! ~ - ref
		POSTFIX,	 // [] () . ++ --
	};

	class Parser;

	typedef Expr* (Parser::* PrefixFn)();
	typedef Expr* (Parser::* InfixFn)(Expr*);

	class Parser
	{
	public:
		Parser();
		~Parser();

		Stmt* Parse(const std::vector<Token>& tokens);

	private:
		void ResetStatus();

		Stmt* ParseAstStmts();
		Stmt* ParseStmt();
		Stmt* ParseExprStmt();
		Stmt* ParseLetStmt();
		Stmt* ParseReturnStmt();
		Stmt* ParseIfStmt();
		Stmt* ParseScopeStmt();
		Stmt* ParseWhileStmt();
		Stmt* ParseClassStmt();

		Expr* ParseExpr(Precedence precedence = Precedence::LOWEST);
		Expr* ParseIdentifierExpr();
		Expr* ParseNumExpr();
		Expr* ParseStrExpr();
		Expr* ParseNilExpr();
		Expr* ParseTrueExpr();
		Expr* ParseFalseExpr();
		Expr* ParseGroupExpr();
		Expr* ParseArrayExpr();
		Expr* ParseTableExpr();
		Expr* ParsePrefixExpr();
		Expr* ParseRefExpr();
		Expr* ParseNewExpr();
		Expr* ParseFunctionExpr();
		Expr* ParseInfixExpr(Expr* prefixExpr);
		Expr* ParseConditionExpr(Expr* prefixExpr);
		Expr* ParseIndexExpr(Expr* prefixExpr);
		Expr* ParseFunctionCallExpr(Expr* prefixExpr);
		Expr* ParseClassCallExpr(Expr* prefixExpr);

		Token GetCurToken();
		Token GetCurTokenAndStepOnce();
		Precedence GetCurTokenPrecedence();

		Token GetNextToken();
		Token GetNextTokenAndStepOnce();
		Precedence GetNextTokenPrecedence();

		bool IsMatchCurToken(TokenType type);
		bool IsMatchCurTokenAndStepOnce(TokenType type);
		bool IsMatchNextToken(TokenType type);
		bool IsMatchNextTokenAndStepOnce(TokenType type);

		Token Consume(TokenType type, std::string_view errMsg);
		Token Consume(const std::vector<TokenType>& types, std::string_view errMsg);

		bool IsAtEnd();

		int64_t m_CurPos;
		AstStmts* m_Stmts;
		std::vector<Token> m_Tokens;

		std::unordered_map<TokenType, PrefixFn> m_PrefixFunctions;
		std::unordered_map<TokenType, InfixFn> m_InfixFunctions;
		std::unordered_map<TokenType, Precedence> m_Precedence;
	};
}