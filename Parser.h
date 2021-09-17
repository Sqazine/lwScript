#pragma once
#include <vector>
#include <cassert>
#include <unordered_map>
#include "Token.h"
#include "Ast.h"
namespace lwScript
{

	enum class Precedence
	{
		LOWEST = 0,
		ASSIGN,		 // = += -= *= /=
		OR,	 // ||
		AND,	 // &&
		EQUAL,		 // == !=
		COMPARE,	 // < <= > >=
		ADD_PLUS,	 // + -
		MUL_DIV, // * / 
		INDEX,// []
		STRUCT_CALL,// .
		FUNCTION_CALL,// ()

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

		Expr* ParseExpr(Precedence precedence = Precedence::LOWEST);
		Expr* ParseIdentifierExpr();
		Expr* ParseNumExpr();
		Expr* ParseStrExpr();
		Expr* ParseNilExpr();
		Expr* ParseTrueExpr();
		Expr* ParseFalseExpr();
		Expr* ParseGroupExpr();
		Expr* ParseFunctionExpr();
		Expr* ParseStructExpr();
		Expr* ParseRefExpr();
		Expr* ParseArrayExpr();
		Expr* ParsePrefixExpr();
		Expr* ParseInfixExpr(Expr* prefixExpr);
		Expr* ParseIndexExpr(Expr* prefixExpr);
		Expr* ParseFunctionCallExpr(Expr* prefixExpr);
		Expr* ParseStructCallExpr(Expr* prefixExpr);

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

		bool IsAtEnd();

		int64_t m_CurPos;
		AstStmts* m_Stmts;
		std::vector<Token> m_Tokens;

		static std::unordered_map<TokenType, PrefixFn> m_PrefixFunctions;
		static std::unordered_map<TokenType, InfixFn> m_InfixFunctions;
		static std::unordered_map<TokenType, Precedence> m_Precedence;
	};
}