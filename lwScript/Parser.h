#pragma once
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Ast.h"
namespace lwScript
{

	enum class Precedence
	{
		LOWEST = 0,
		ASSIGN,		   // = += -= *= /=
		OR,			   // ||
		AND,		   // &&
		EQUAL,		   // == !=
		COMPARE,	   // < <= > >=
		ADD_PLUS,	   // + -
		MUL_DIV,	   // * /
		INDEX,		   // []
		FUNCTION_CALL, // ()

	};

	class Parser;

	typedef Expr *(Parser::*PrefixFn)();
	typedef Expr *(Parser::*InfixFn)(Expr *);

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
		Stmt *ParseReturnStmt();
		Stmt *ParseIfStmt();
		Stmt *ParseScopeStmt();
		Stmt *ParseWhileStmt();

		Expr *ParseExpr(Precedence precedence = Precedence::LOWEST);
		Expr *ParseIdentifierExpr();
		Expr *ParseNumExpr();
		Expr *ParseStrExpr();
		Expr *ParseNilExpr();
		Expr *ParseTrueExpr();
		Expr *ParseFalseExpr();
		Expr *ParseGroupExpr();
		Expr *ParseFunctionExpr();
		Expr *ParseArrayExpr();
		Expr *ParsePrefixExpr();
		Expr *ParseInfixExpr(Expr *prefixExpr);
		Expr *ParseIndexExpr(Expr *prefixExpr);
		Expr *ParseFunctionCallExpr(Expr *prefixExpr);

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
		AstStmts *m_Stmts;
		std::vector<Token> m_Tokens;

		std::unordered_map<TokenType, PrefixFn> m_PrefixFunctions;
		std::unordered_map<TokenType, InfixFn> m_InfixFunctions;
		std::unordered_map<TokenType, Precedence> m_Precedence;
	};

	Parser::Parser()
		: m_Stmts(nullptr)
	{
		m_PrefixFunctions =
		{
			{TokenType::IDENTIFIER, &Parser::ParseIdentifierExpr},
			{TokenType::NUMBER, &Parser::ParseNumExpr},
			{TokenType::STRING, &Parser::ParseStrExpr},
			{TokenType::NIL, &Parser::ParseNilExpr},
			{TokenType::TRUE, &Parser::ParseTrueExpr},
			{TokenType::FALSE, &Parser::ParseFalseExpr},
			{TokenType::MINUS, &Parser::ParsePrefixExpr},
			{TokenType::LPAREN, &Parser::ParseGroupExpr},
			{TokenType::FUNCTION, &Parser::ParseFunctionExpr},
			{TokenType::LBRACKET, &Parser::ParseArrayExpr},

	};

	m_InfixFunctions =
		{
			{TokenType::EQUAL, &Parser::ParseInfixExpr},
			{TokenType::OR, &Parser::ParseInfixExpr},
			{TokenType::AND, &Parser::ParseInfixExpr},
			{TokenType::EEQUAL, &Parser::ParseInfixExpr},
			{TokenType::BEQUAL, &Parser::ParseInfixExpr},
			{TokenType::LESS, &Parser::ParseInfixExpr},
			{TokenType::LEQUAL, &Parser::ParseInfixExpr},
			{TokenType::GREATER, &Parser::ParseInfixExpr},
			{TokenType::GEQUAL, &Parser::ParseInfixExpr},
			{TokenType::PLUS, &Parser::ParseInfixExpr},
			{TokenType::MINUS, &Parser::ParseInfixExpr},
			{TokenType::ASTERISK, &Parser::ParseInfixExpr},
			{TokenType::SLASH, &Parser::ParseInfixExpr},
			{TokenType::LPAREN, &Parser::ParseFunctionCallExpr},
			{TokenType::LBRACKET, &Parser::ParseIndexExpr},
	};

	m_Precedence =
		{

			{TokenType::EQUAL, Precedence::ASSIGN},
			{TokenType::OR, Precedence::OR},
			{TokenType::AND, Precedence::AND},
			{TokenType::EEQUAL, Precedence::EQUAL},
			{TokenType::BEQUAL, Precedence::EQUAL},
			{TokenType::LESS, Precedence::COMPARE},
			{TokenType::LEQUAL, Precedence::COMPARE},
			{TokenType::GREATER, Precedence::COMPARE},
			{TokenType::GEQUAL, Precedence::COMPARE},
			{TokenType::PLUS, Precedence::ADD_PLUS},
			{TokenType::MINUS, Precedence::ADD_PLUS},
			{TokenType::ASTERISK, Precedence::MUL_DIV},
			{TokenType::SLASH, Precedence::MUL_DIV},
			{TokenType::LBRACKET, Precedence::INDEX},
			{TokenType::LPAREN, Precedence::FUNCTION_CALL},
	};

	}
	Parser::~Parser()
	{
	}

	Stmt *Parser::Parse(const std::vector<Token> &tokens)
	{
		ResetStatus();
		m_Tokens = tokens;

		return ParseAstStmts();
	}

	void Parser::ResetStatus()
	{
		m_CurPos = 0;

		if (m_Stmts != nullptr)
		{
			delete m_Stmts;
			m_Stmts = nullptr;
		}
		m_Stmts = new AstStmts();
	}

	Stmt *Parser::ParseAstStmts()
	{
		while (!IsMatchCurToken(TokenType::END))
			m_Stmts->stmts.emplace_back(ParseStmt());
		return m_Stmts;
	}

	Stmt *Parser::ParseStmt()
	{
		if (IsMatchCurToken(TokenType::LET))
			return ParseLetStmt();
		else if (IsMatchCurToken(TokenType::RETURN))
			return ParseReturnStmt();
		else if (IsMatchCurToken(TokenType::IF))
			return ParseIfStmt();
		else if (IsMatchCurToken(TokenType::LBRACE))
			return ParseScopeStmt();
		else if (IsMatchCurToken(TokenType::WHILE))
			return ParseWhileStmt();
		else
			return ParseExprStmt();
	}

	Stmt *Parser::ParseExprStmt()
	{
		auto exprStmt = new ExprStmt(ParseExpr());
		Consume(TokenType::SEMICOLON, "Expect ';' after expr stmt.");
		return exprStmt;
	}

	Stmt *Parser::ParseLetStmt()
	{
		Consume(TokenType::LET, "Expect 'let' key word");
		auto letStmt = new LetStmt();

		//the first variable
		auto identifier = new IdentifierExpr(Consume(TokenType::IDENTIFIER, "Expect valid identifier").literal);
		Expr *value = nilExpr;
		if (IsMatchCurTokenAndStepOnce(TokenType::EQUAL))
			value = ParseExpr();
		letStmt->variables[identifier] = value;

		//other variable
		while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
		{
			identifier = new IdentifierExpr(Consume(TokenType::IDENTIFIER, "Expect valid identifier").literal);
			Expr *value = nilExpr;
			if (IsMatchCurTokenAndStepOnce(TokenType::EQUAL))
				value = ParseExpr();
			letStmt->variables[identifier] = value;
		}

		Consume(TokenType::SEMICOLON, "Expect ';' after let stmt.");

		return letStmt;
	}

	Stmt *Parser::ParseReturnStmt()
	{
		Consume(TokenType::RETURN, "Expect 'return' key word.");

		auto returnStmt = new ReturnStmt();

		if (!IsMatchCurToken(TokenType::SEMICOLON))
			returnStmt->expr = ParseExpr();

		Consume(TokenType::SEMICOLON, "Expect ';' after return stmt");
		return returnStmt;
	}

	Stmt *Parser::ParseIfStmt()
	{
		Consume(TokenType::IF, "Expect 'if' key word.");
		Consume(TokenType::LPAREN, "Expect '(' after 'if'.");

		auto ifStmt = new IfStmt();

		ifStmt->condition = ParseExpr();

		Consume(TokenType::RPAREN, "Expect ')' after if condition");

		ifStmt->thenBranch = ParseStmt();

		if (IsMatchCurTokenAndStepOnce(TokenType::ELSE))
			ifStmt->elseBranch = ParseStmt();

		return ifStmt;
	}

	Stmt *Parser::ParseScopeStmt()
	{
		Consume(TokenType::LBRACE, "Expect '{'.");
		auto scopeStmt = new ScopeStmt();
		while (!IsMatchCurToken(TokenType::RBRACE))
			scopeStmt->stmts.emplace_back(ParseStmt());
		Consume(TokenType::RBRACE, "Expect '}'.");
		return scopeStmt;
	}

	Stmt *Parser::ParseWhileStmt()
	{
		Consume(TokenType::WHILE, "Expect 'while' keyword.");
		Consume(TokenType::LPAREN, "Expect '(' after 'while'.");

		auto whileStmt = new WhileStmt();

		whileStmt->condition = ParseExpr();

		Consume(TokenType::RPAREN, "Expect ')' after while stmt's condition.");

		whileStmt->body = ParseStmt();

		return whileStmt;
	}

	Expr *Parser::ParseExpr(Precedence precedence)
	{
		if (m_PrefixFunctions.find(GetCurToken().type) == m_PrefixFunctions.end())
		{
			std::cout << "no prefix definition for:" << GetCurTokenAndStepOnce().literal << std::endl;
			return nilExpr;
		}
		auto prefixFn = m_PrefixFunctions[GetCurToken().type];

		auto leftExpr = (this->*prefixFn)();

		while (!IsMatchCurToken(TokenType::SEMICOLON) && precedence < GetCurTokenPrecedence())
		{
			if (m_InfixFunctions.find(GetCurToken().type) == m_InfixFunctions.end())
				return leftExpr;

			auto infixFn = m_InfixFunctions[GetCurToken().type];

			leftExpr = (this->*infixFn)(leftExpr);
		}

		return leftExpr;
	}

	Expr *Parser::ParseIdentifierExpr()
	{
		return new IdentifierExpr(Consume(TokenType::IDENTIFIER, "Expect a identifier.").literal);
	}

	Expr *Parser::ParseNumExpr()
	{
		std::string numLiteral = Consume(TokenType::NUMBER, "Expexct a number literal.").literal;

		return new NumExpr(std::stod(numLiteral));
	}

	Expr *Parser::ParseStrExpr()
	{
		return new StrExpr(Consume(TokenType::STRING, "Expect a string literal.").literal);
	}

	Expr *Parser::ParseNilExpr()
	{
		Consume(TokenType::NIL, "Expect 'null' keyword");
		return nilExpr;
	}
	Expr *Parser::ParseTrueExpr()
	{
		Consume(TokenType::TRUE, "Expect 'true' keyword");
		return trueExpr;
	}
	Expr *Parser::ParseFalseExpr()
	{
		Consume(TokenType::FALSE, "Expect 'false' keyword");
		return falseExpr;
	}

	Expr *Parser::ParseGroupExpr()
	{
		Consume(TokenType::LPAREN, "Expect '('.");
		auto groupExpr = new GroupExpr(ParseExpr());
		Consume(TokenType::RPAREN, "Expect ')'.");
		return groupExpr;
	}

	Expr *Parser::ParseFunctionExpr()
	{
		Consume(TokenType::FUNCTION, "Expect 'function' keyword");
		Consume(TokenType::LPAREN, "Expect '(' after 'function' keyword");
		auto funcExpr = new FunctionExpr();

		if (!IsMatchCurToken(TokenType::RPAREN)) //has parameter
		{
			IdentifierExpr *idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
			funcExpr->parameters.emplace_back(idenExpr);
			while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
			{
				idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
				funcExpr->parameters.emplace_back(idenExpr);
			}
		}
		Consume(TokenType::RPAREN, "Expect ')' after function expr's '('");

		funcExpr->body = (ScopeStmt *)ParseScopeStmt();

		return funcExpr;
	}

	Expr *Parser::ParseArrayExpr()
	{
		Consume(TokenType::LBRACKET, "Expect '['.");

		auto arrayExpr = new ArrayExpr();
		if (!IsMatchCurToken(TokenType::RBRACKET))
		{
			//first element
			arrayExpr->elements.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
				arrayExpr->elements.emplace_back(ParseExpr());
		}

		Consume(TokenType::RBRACKET, "Expect ']'.");

		return arrayExpr;
	}

	Expr *Parser::ParsePrefixExpr()
	{
		auto prefixExpr = new PrefixExpr();
		prefixExpr->op = GetCurTokenAndStepOnce().literal;
		prefixExpr->right = ParseExpr();
		return prefixExpr;
	}

	Expr *Parser::ParseInfixExpr(Expr *prefixExpr)
	{
		auto infixExpr = new InfixExpr();
		infixExpr->left = prefixExpr;
		infixExpr->op = GetCurTokenAndStepOnce().literal;
		infixExpr->right = ParseExpr(GetCurTokenPrecedence());
		return infixExpr;
	}

	Expr *Parser::ParseIndexExpr(Expr *prefixExpr)
	{
		Consume(TokenType::LBRACKET, "Expect '['.");
		auto indexExpr = new IndexExpr();
		indexExpr->array = prefixExpr;
		indexExpr->index = ParseExpr();
		Consume(TokenType::RBRACKET, "Expect ']'.");
		return indexExpr;
	}

	Expr *Parser::ParseFunctionCallExpr(Expr *prefixExpr)
	{
		auto funcCallExpr = new FunctionCallExpr();

		funcCallExpr->name = prefixExpr;
		Consume(TokenType::LPAREN, "Expect '('.");
		if (!IsMatchCurToken(TokenType::RPAREN)) //has arguments
		{
			funcCallExpr->arguments.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
				funcCallExpr->arguments.emplace_back(ParseExpr());
		}
		Consume(TokenType::RPAREN, "Expect ')'.");

		return funcCallExpr;
	}

	Token Parser::GetCurToken()
	{
		if (!IsAtEnd())
			return m_Tokens[m_CurPos];
		return m_Tokens.back();
	}
	Token Parser::GetCurTokenAndStepOnce()
	{
		if (!IsAtEnd())
			return m_Tokens[m_CurPos++];
		return m_Tokens.back();
	}

	Precedence Parser::GetCurTokenPrecedence()
	{
		if (m_Precedence.find(GetCurToken().type) != m_Precedence.end())
			return m_Precedence[GetCurToken().type];
		return Precedence::LOWEST;
	}

	Token Parser::GetNextToken()
	{
		if (m_CurPos + 1 < (int32_t)m_Tokens.size())
			return m_Tokens[m_CurPos + 1];
		return m_Tokens.back();
	}
	Token Parser::GetNextTokenAndStepOnce()
	{
		if (m_CurPos + 1 < (int32_t)m_Tokens.size())
			return m_Tokens[++m_CurPos];
		return m_Tokens.back();
	}

	Precedence Parser::GetNextTokenPrecedence()
	{
		if (m_Precedence.find(GetNextToken().type) != m_Precedence.end())
			return m_Precedence[GetNextToken().type];
		return Precedence::LOWEST;
	}

	bool Parser::IsMatchCurToken(TokenType type)
	{
		return GetCurToken().type == type;
	}

	bool Parser::IsMatchCurTokenAndStepOnce(TokenType type)
	{
		if (IsMatchCurToken(type))
		{
			m_CurPos++;
			return true;
		}
		return false;
	}

	bool Parser::IsMatchNextToken(TokenType type)
	{
		return GetNextToken().type == type;
	}

	bool Parser::IsMatchNextTokenAndStepOnce(TokenType type)
	{
		if (IsMatchNextToken(type))
		{
			m_CurPos++;
			return true;
		}
		return false;
	}

	Token Parser::Consume(TokenType type, std::string_view errMsg)
	{
		if (IsMatchCurToken(type))
			return GetCurTokenAndStepOnce();
		std::cout << "[line " << std::to_string(GetCurToken().line) + "]:" << errMsg << std::endl;
		exit(1);
	}

	bool Parser::IsAtEnd()
	{
		return m_CurPos >= (int32_t)m_Tokens.size();
	}
}