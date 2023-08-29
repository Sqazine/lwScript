#pragma once
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Ast.h"
#include "Utils.h"

#include "Optimizer.h"
#include "SyntaxChecker.h"
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
		INFIX,		 // [] () . !
		POSTFIX,	 // ++ -- !
	};

	enum class Associativity
	{
		L2R, //left->right
		R2L	 //right->left
	};

	struct ClassInfo
	{
		ClassInfo *enclosing;
		bool hasSuperClass;
		std::wstring name;
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

		Stmt *ParseDecl();
		Stmt *ParseVarDecl(TokenType tType);
		Stmt *ParseFunctionDecl();
		Stmt *ParseClassDecl();
		Stmt *ParseEnumDecl();
		Stmt *ParseModuleDecl();

		Stmt *ParseStmt();
		Stmt *ParseExprStmt();
		Stmt *ParseReturnStmt();
		Stmt *ParseIfStmt();
		Stmt *ParseScopeStmt();
		Stmt *ParseWhileStmt();
		Stmt *ParseForStmt();
		Stmt *ParseBreakStmt();
		Stmt *ParseContinueStmt();
		Stmt *ParseSwitchStmt();

		Expr *ParseExpr(Precedence precedence = Precedence::LOWEST);
		Expr *ParseIdentifierExpr();
		Expr *ParseNumExpr();
		Expr *ParseStrExpr();
		Expr *ParseNullExpr();
		Expr *ParseTrueExpr();
		Expr *ParseFalseExpr();
		Expr *ParseGroupExpr();
		Expr *ParseArrayExpr();
		Expr *ParseDictExpr();
		Expr *ParseAnonyObjExpr();
		Expr *ParsePrefixExpr();
		Expr *ParseRefExpr();
		Expr *ParseLambdaExpr();
		Expr *ParseNewExpr();
		Expr *ParseThisExpr();
		Expr *ParseBaseExpr();
		Expr *ParseMatchExpr();
		Expr *ParseBlockExpr();
		Expr *ParseInfixExpr(Expr *prefixExpr);
		Expr *ParsePostfixExpr(Expr *prefixExpr);
		Expr *ParseConditionExpr(Expr *prefixExpr);
		Expr *ParseIndexExpr(Expr *prefixExpr);
		Expr *ParseCallExpr(Expr *prefixExpr);
		Expr *ParseDotExpr(Expr *prefixExpr);
		Expr *ParseFactorialExpr(Expr *prefixExpr);
		Expr *ParseVarDescExpr();
		Expr *ParseVarArgExpr();
		std::pair<Expr *, Expr *> ParseDestructuringAssignmentExpr();

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

		Optimizer mOptimizer;
		SyntaxChecker mSyntaxChecker;

		ClassInfo *mCurClassInfo;

		std::vector<TokenType> mSkippingConsumeTokenTypeStack; //skip token while call consume function

		int64_t mCurPos;
		std::vector<Token> mTokens;

		static std::unordered_map<TokenType, PrefixFn> mPrefixFunctions;
		static std::unordered_map<TokenType, InfixFn> mInfixFunctions;
		static std::unordered_map<TokenType, PostfixFn> mPostfixFunctions;

		static NullExpr *mNullExpr;
	};
}