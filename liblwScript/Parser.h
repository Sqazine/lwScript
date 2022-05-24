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
		LOWEST = 0,	 // ,
		ASSIGN,		 // = += -= *= /= %= &= |= ^= <<= >>=
		CONDITION,	 // ?:
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
		PREFIX,		 // ! ~ - & ++ --
		INFIX,		 // [] () .
		POSTFIX,	 // ++ --
	};

	enum class Associativity
	{
		L2R, //left->right
		R2L	 //right->left
	};

	class Parser;

	typedef Expr *(Parser::*PrefixFn)();
	typedef Expr *(Parser::*InfixFn)(Expr *);
	typedef Expr *(Parser::*PostfixFn)(Expr *);

	class Parser
	{
	public:
		Parser();
		~Parser();

		Stmt *Parse(const std::vector<Token> &tokens);

	private:
		void ResetStatus();

		Stmt *ParseAstStmts();
		Stmt *ParseStmt();
		Stmt *ParseExprStmt();
		Stmt *ParseLetStmt();
		Stmt *ParseConstStmt();
		Stmt *ParseReturnStmt();
		Stmt *ParseIfStmt();
		Stmt *ParseScopeStmt();
		Stmt *ParseWhileStmt();
		Stmt *ParseForStmt();
		Stmt *ParseBreakStmt();
		Stmt *ParseContinueStmt();
		Stmt *ParseSwitchStmt();
		Stmt *ParseMatchStmt();
		Stmt *ParseEnumStmt();
		Stmt *ParseFunctionStmt();
		Stmt *ParseClassStmt();

		Expr *ParseExpr(Precedence precedence = Precedence::LOWEST);
		Expr *ParseIdentifierExpr();
		Expr *ParseNumExpr();
		Expr *ParseStrExpr();
		Expr *ParseNullExpr();
		Expr *ParseTrueExpr();
		Expr *ParseFalseExpr();
		Expr *ParseGroupExpr();
		Expr *ParseArrayExpr();
		Expr *ParseTableExpr();
		Expr *ParsePrefixExpr();
		Expr *ParseRefExpr();
		Expr *ParseLambdaExpr();
		Expr *ParseInfixExpr(Expr *prefixExpr);
		Expr *ParsePostfixExpr(Expr *prefixExpr);
		Expr *ParseConditionExpr(Expr *prefixExpr);
		Expr *ParseIndexExpr(Expr *prefixExpr);
		Expr *ParseFunctionCallExpr(Expr *prefixExpr);
		Expr *ParseClassCallExpr(Expr *prefixExpr);

		Token GetCurToken();
		Token GetCurTokenAndStepOnce();
		Precedence GetCurTokenPrecedence();
		Associativity GetCurTokenAssociativity();

		Token GetNextToken();
		Token GetNextTokenAndStepOnce();
		Precedence GetNextTokenPrecedence();

		bool IsMatchCurToken(TokenType type);
		bool IsMatchCurTokenAndStepOnce(TokenType type);
		bool IsMatchNextToken(TokenType type);
		bool IsMatchNextTokenAndStepOnce(TokenType type);

		Token Consume(TokenType type, std::wstring_view errMsg);
		Token Consume(const std::vector<TokenType> &types, std::wstring_view errMsg);

		bool IsAtEnd();

		int64_t mCurPos;
		AstStmts *mStmts;
		std::vector<Token> mTokens;

		static std::unordered_map<TokenType, PrefixFn> mPrefixFunctions;
		static std::unordered_map<TokenType, InfixFn> mInfixFunctions;
		static std::unordered_map<TokenType, PostfixFn> mPostfixFunctions;
	};
}