#include "Parser.h"
namespace lws
{
	std::unordered_map<TokenType, PrefixFn> Parser::m_PrefixFunctions =
	{
		{TokenType::IDENTIFIER, &Parser::ParseIdentifierExpr},
		{TokenType::NUMBER, &Parser::ParseNumExpr},
		{TokenType::STRING, &Parser::ParseStrExpr},
		{TokenType::NIL, &Parser::ParseNilExpr},
		{TokenType::TRUE, &Parser::ParseTrueExpr},
		{TokenType::FALSE, &Parser::ParseFalseExpr},
		{TokenType::MINUS, &Parser::ParsePrefixExpr},
		{TokenType::TILDE, &Parser::ParsePrefixExpr},
		{TokenType::BANG, &Parser::ParsePrefixExpr},
		{TokenType::LPAREN, &Parser::ParseGroupExpr},
		{TokenType::LBRACKET, &Parser::ParseArrayExpr},
		{TokenType::LBRACE, &Parser::ParseTableExpr},
		{TokenType::AMPERSAND, &Parser::ParseRefExpr},
		{TokenType::LAMBDA,&Parser::ParseLambdaExpr},
		{TokenType::NEW,&Parser::ParseNewExpr}
	};

	std::unordered_map<TokenType, InfixFn> Parser::m_InfixFunctions =
	{
		{TokenType::EQUAL, &Parser::ParseInfixExpr},
		{TokenType::PLUS_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::MINUS_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::ASTERISK_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::SLASH_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::PERCENT_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::AMPERSAND_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::VBAR_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::CARET_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::LESS_LESS_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::GREATER_GREATER_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::QUESTION, &Parser::ParseConditionExpr},
		{TokenType::VBAR_VBAR, &Parser::ParseInfixExpr},
		{TokenType::AMPERSAND_AMPERSAND, &Parser::ParseInfixExpr},
		{TokenType::VBAR, &Parser::ParseInfixExpr},
		{TokenType::CARET, &Parser::ParseInfixExpr},
		{TokenType::AMPERSAND, &Parser::ParseInfixExpr},
		{TokenType::LESS_LESS, &Parser::ParseInfixExpr},
		{TokenType::GREATER_GREATER, &Parser::ParseInfixExpr},
		{TokenType::EQUAL_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::BANG_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::LESS, &Parser::ParseInfixExpr},
		{TokenType::LESS_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::GREATER, &Parser::ParseInfixExpr},
		{TokenType::GREATER_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::PLUS, &Parser::ParseInfixExpr},
		{TokenType::MINUS, &Parser::ParseInfixExpr},
		{TokenType::ASTERISK, &Parser::ParseInfixExpr},
		{TokenType::SLASH, &Parser::ParseInfixExpr},
		{TokenType::PERCENT, &Parser::ParseInfixExpr},
		{TokenType::LPAREN, &Parser::ParseFunctionCallExpr},
		{TokenType::LBRACKET, &Parser::ParseIndexExpr},
		{TokenType::DOT, &Parser::ParseClassCallExpr},
	};

	std::unordered_map<TokenType, Precedence> Parser::m_Precedence =
	{
		{TokenType::EQUAL, Precedence::ASSIGN},
		{TokenType::PLUS_EQUAL, Precedence::ASSIGN},
		{TokenType::MINUS_EQUAL, Precedence::ASSIGN},
		{TokenType::ASTERISK_EQUAL, Precedence::ASSIGN},
		{TokenType::SLASH_EQUAL, Precedence::ASSIGN},
		{TokenType::PERCENT_EQUAL, Precedence::ASSIGN},
		{TokenType::AMPERSAND_EQUAL, Precedence::ASSIGN},
		{TokenType::VBAR_EQUAL, Precedence::ASSIGN},
		{TokenType::CARET_EQUAL, Precedence::ASSIGN},
		{TokenType::LESS_LESS_EQUAL, Precedence::ASSIGN},
		{TokenType::GREATER_GREATER_EQUAL, Precedence::ASSIGN},
		{TokenType::VBAR_VBAR, Precedence::OR},
		{TokenType::AMPERSAND_AMPERSAND, Precedence::AND},
		{TokenType::QUESTION, Precedence::CONDITION},
		{TokenType::VBAR, Precedence::BIT_OR},
		{TokenType::CARET, Precedence::BIT_XOR},
		{TokenType::AMPERSAND, Precedence::BIT_AND},
		{TokenType::EQUAL_EQUAL, Precedence::EQUAL},
		{TokenType::BANG_EQUAL, Precedence::EQUAL},
		{TokenType::LESS, Precedence::COMPARE},
		{TokenType::LESS_EQUAL, Precedence::COMPARE},
		{TokenType::GREATER, Precedence::COMPARE},
		{TokenType::GREATER_EQUAL, Precedence::COMPARE},
		{TokenType::LESS_LESS, Precedence::BIT_SHIFT},
		{TokenType::GREATER_GREATER, Precedence::BIT_SHIFT},
		{TokenType::PLUS, Precedence::ADD_PLUS},
		{TokenType::MINUS, Precedence::ADD_PLUS},
		{TokenType::ASTERISK, Precedence::MUL_DIV_MOD},
		{TokenType::SLASH, Precedence::MUL_DIV_MOD},
		{TokenType::PERCENT, Precedence::MUL_DIV_MOD},
		{TokenType::LBRACKET, Precedence::INFIX},
		{TokenType::LPAREN, Precedence::INFIX},
		{TokenType::DOT, Precedence::INFIX},
	};

	std::unordered_map<Precedence, Associativity> Parser::m_Associativity =
	{
		{Precedence::LOWEST,Associativity::LEFT2RIGHT},
		{Precedence::ASSIGN,Associativity::RIGHT2LEFT},
		{Precedence::CONDITION,Associativity::LEFT2RIGHT},
		{Precedence::OR,Associativity::LEFT2RIGHT},
		{Precedence::AND,Associativity::LEFT2RIGHT},
		{Precedence::BIT_OR,Associativity::LEFT2RIGHT},
		{Precedence::BIT_XOR,Associativity::LEFT2RIGHT},
		{Precedence::BIT_AND,Associativity::LEFT2RIGHT},
		{Precedence::EQUAL,Associativity::LEFT2RIGHT},
		{Precedence::COMPARE,Associativity::LEFT2RIGHT},
		{Precedence::BIT_SHIFT,Associativity::LEFT2RIGHT},
		{Precedence::ADD_PLUS,Associativity::LEFT2RIGHT},
		{Precedence::MUL_DIV_MOD,Associativity::LEFT2RIGHT},
		{Precedence::PREFIX,Associativity::RIGHT2LEFT},
		{Precedence::INFIX,Associativity::LEFT2RIGHT},

	};

	Parser::Parser()
		: m_Stmts(nullptr)
	{

	}
	Parser::~Parser()
	{
		if (m_Stmts != nullptr)
		{
			delete m_Stmts;
			m_Stmts = nullptr;
		}

		std::unordered_map<TokenType, PrefixFn>().swap(m_PrefixFunctions);
		std::unordered_map<TokenType, InfixFn>().swap(m_InfixFunctions);
		std::unordered_map<TokenType, Precedence>().swap(m_Precedence);
	}

	Stmt* Parser::Parse(const std::vector<Token>& tokens)
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

	Stmt* Parser::ParseAstStmts()
	{
		while (!IsMatchCurToken(TokenType::END))
			m_Stmts->stmts.emplace_back(ParseStmt());
		return m_Stmts;
	}

	Stmt* Parser::ParseStmt()
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
		else if (IsMatchCurToken(TokenType::FOR))
			return ParseForStmt();
		else  if (IsMatchCurToken(TokenType::FUNCTION))
			return ParseFunctionStmt();
		else if (IsMatchCurToken(TokenType::CLASS))
			return ParseClassStmt();
		else
			return ParseExprStmt();
	}

	Stmt* Parser::ParseExprStmt()
	{
		auto exprStmt = new ExprStmt(ParseExpr());
		Consume(TokenType::SEMICOLON, "Expect ';' after expr stmt.");
		return exprStmt;
	}

	Stmt* Parser::ParseLetStmt()
	{
		Consume(TokenType::LET, "Expect 'let' key word");

		std::unordered_map<IdentifierExpr*, Expr*> variables;

		auto name = (IdentifierExpr*)ParseIdentifierExpr();
		Expr* value = nilExpr;
		if (IsMatchCurTokenAndStepOnce(TokenType::EQUAL))
			value = ParseExpr();
		variables[name] = value;

		while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
		{
			auto name = (IdentifierExpr*)ParseIdentifierExpr();
			Expr* value = nilExpr;
			if (IsMatchCurTokenAndStepOnce(TokenType::EQUAL))
				value = ParseExpr();
			variables[name] = value;
		}

		Consume(TokenType::SEMICOLON, "Expect ';' after let stmt.");

		return new LetStmt(variables);
	}

	Stmt* Parser::ParseReturnStmt()
	{
		Consume(TokenType::RETURN, "Expect 'return' key word.");

		auto returnStmt = new ReturnStmt();

		if (!IsMatchCurToken(TokenType::SEMICOLON))
			returnStmt->expr = ParseExpr();

		Consume(TokenType::SEMICOLON, "Expect ';' after return stmt");
		return returnStmt;
	}

	Stmt* Parser::ParseIfStmt()
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

	Stmt* Parser::ParseScopeStmt()
	{
		Consume(TokenType::LBRACE, "Expect '{'.");
		auto scopeStmt = new ScopeStmt();
		while (!IsMatchCurToken(TokenType::RBRACE))
			scopeStmt->stmts.emplace_back(ParseStmt());
		Consume(TokenType::RBRACE, "Expect '}'.");
		return scopeStmt;
	}

	Stmt* Parser::ParseWhileStmt()
	{
		Consume(TokenType::WHILE, "Expect 'while' keyword.");
		Consume(TokenType::LPAREN, "Expect '(' after 'while'.");

		auto whileStmt = new WhileStmt();

		whileStmt->condition = ParseExpr();

		Consume(TokenType::RPAREN, "Expect ')' after while stmt's condition.");

		whileStmt->body = ParseStmt();

		return whileStmt;
	}

	Stmt* Parser::ParseForStmt()
	{
		//for(let i=0,j=0;i<10&&j<10;i=i+1,j=j+1)
		//{
		//	...
		//}
		// |
		// |
		// v
		//{
		//		let i=0,j=0;
		//		while(i<10&&j<10)
		//		{
		//			...
		//			i=i+1;
		//			j=j+1;
		//		}
		//}
		Consume(TokenType::FOR, "Expect 'for' keyword.");
		Consume(TokenType::LPAREN, "Expect '(' after 'for'.");

		//initializer
		auto scopeStmt = new ScopeStmt();
		if (IsMatchCurToken(TokenType::LET))
			scopeStmt->stmts.emplace_back(ParseLetStmt());
		else
		{
			if (!IsMatchCurToken(TokenType::SEMICOLON))
			{
				scopeStmt->stmts.emplace_back(new ExprStmt(ParseExpr()));
				while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
					scopeStmt->stmts.emplace_back(new ExprStmt(ParseExpr()));
			}
			Consume(TokenType::SEMICOLON, "Expect ';' after for stmt's initializer stmt");
		}

		Expr* condition = nullptr;
		if (!IsMatchCurToken(TokenType::SEMICOLON))
			condition = ParseExpr();
		Consume(TokenType::SEMICOLON, "Expect ';' after for stmt's condition expr.");

		std::vector<Expr* > increment;
		if (!IsMatchCurToken(TokenType::RPAREN))
		{
			increment.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
				increment.emplace_back(ParseExpr());
		}
		Consume(TokenType::RPAREN, "Expect ')' after for stmt's increment expr(s)");

		std::vector<Stmt*> whileBodyStmts;
		whileBodyStmts.emplace_back(ParseStmt());
		for (const auto expr : increment)
			whileBodyStmts.emplace_back(new ExprStmt(expr));

		auto whileStmt = new WhileStmt();
		whileStmt->condition = condition;
		whileStmt->body = new ScopeStmt(whileBodyStmts);

		scopeStmt->stmts.emplace_back(whileStmt);

		return scopeStmt;
	}

	Stmt* Parser::ParseFunctionStmt()
	{
		Consume(TokenType::FUNCTION, "Expect 'function' keyword");

		auto funcStmt = new FunctionStmt();

		funcStmt->name =(IdentifierExpr*)ParseIdentifierExpr();

		Consume(TokenType::LPAREN, "Expect '(' after 'function' keyword");

		if (!IsMatchCurToken(TokenType::RPAREN)) //has parameter
		{
			IdentifierExpr* idenExpr = (IdentifierExpr*)ParseIdentifierExpr();
			funcStmt->parameters.emplace_back(idenExpr);
			while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
			{
				idenExpr = (IdentifierExpr*)ParseIdentifierExpr();
				funcStmt->parameters.emplace_back(idenExpr);
			}
		}
		Consume(TokenType::RPAREN, "Expect ')' after function stmt's '('");

		funcStmt->body = (ScopeStmt*)ParseScopeStmt();

		return funcStmt;
	}

	Expr* Parser::ParseLambdaExpr()
	{
		Consume(TokenType::LAMBDA, "Expect 'lambda' keyword");

		auto funcExpr = new LambdaExpr();

		Consume(TokenType::LPAREN, "Expect '(' after 'lambda' keyword");

		if (!IsMatchCurToken(TokenType::RPAREN)) //has parameter
		{
			IdentifierExpr* idenExpr = (IdentifierExpr*)ParseIdentifierExpr();
			funcExpr->parameters.emplace_back(idenExpr);
			while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
			{
				idenExpr = (IdentifierExpr*)ParseIdentifierExpr();
				funcExpr->parameters.emplace_back(idenExpr);
			}
		}
		Consume(TokenType::RPAREN, "Expect ')' after lambda expr's '('");

		funcExpr->body = (ScopeStmt*)ParseScopeStmt();

		return funcExpr;
	}

	Stmt* Parser::ParseClassStmt()
	{
		Consume(TokenType::CLASS, "Expect 'class' keyword");

		auto structStmt = new ClassStmt();
		structStmt->name = ((IdentifierExpr*)ParseIdentifierExpr())->literal;

		Consume(TokenType::LBRACE, "Expect '{' after 'class' keyword");

		while (!IsMatchCurToken(TokenType::RBRACE))
		{
			if (IsMatchCurToken(TokenType::LET))
				structStmt->letStmts.emplace_back((LetStmt*)ParseLetStmt());
			else Consume(TokenType::LET, "UnExpect identifier '" + GetCurToken().literal + "'.");
		}

		Consume(TokenType::RBRACE, "Expect '}' after class stmt's '{'");

		return structStmt;
	}

	Expr* Parser::ParseExpr(Precedence precedence)
	{
		if (m_PrefixFunctions.find(GetCurToken().type) == m_PrefixFunctions.end())
		{
			std::cout << "no prefix definition for:" << GetCurTokenAndStepOnce().literal << std::endl;
			return nilExpr;
		}
		auto prefixFn = m_PrefixFunctions[GetCurToken().type];

		auto leftExpr = (this->*prefixFn)();

		while (!IsMatchCurToken(TokenType::SEMICOLON) &&(GetCurTokenAssociativity()==Associativity::LEFT2RIGHT? precedence < GetCurTokenPrecedence(): precedence <= GetCurTokenPrecedence()))
		{
			if (m_InfixFunctions.find(GetCurToken().type) == m_InfixFunctions.end())
				return leftExpr;

			auto infixFn = m_InfixFunctions[GetCurToken().type];

			leftExpr = (this->*infixFn)(leftExpr);
		}

		return leftExpr;
	}

	Expr* Parser::ParseIdentifierExpr()
	{
		return new IdentifierExpr(Consume(TokenType::IDENTIFIER, "Unexpect Identifier'" + GetCurToken().literal + "'.").literal);
	}

	Expr* Parser::ParseNumExpr()
	{
		std::string numLiteral = Consume(TokenType::NUMBER, "Expexct a number literal.").literal;
		if (numLiteral.find('.') != std::string::npos)
			return new FloatingExpr(std::stod(numLiteral));
		else
			return new IntegerExpr(std::stoll(numLiteral));
	}

	Expr* Parser::ParseStrExpr()
	{
		return new StrExpr(Consume(TokenType::STRING, "Expect a string literal.").literal);
	}

	Expr* Parser::ParseNilExpr()
	{
		Consume(TokenType::NIL, "Expect 'null' keyword");
		return nilExpr;
	}
	Expr* Parser::ParseTrueExpr()
	{
		Consume(TokenType::TRUE, "Expect 'true' keyword");
		return trueExpr;
	}
	Expr* Parser::ParseFalseExpr()
	{
		Consume(TokenType::FALSE, "Expect 'false' keyword");
		return falseExpr;
	}

	Expr* Parser::ParseGroupExpr()
	{
		Consume(TokenType::LPAREN, "Expect '('.");
		auto groupExpr = new GroupExpr(ParseExpr(Precedence::INFIX));
		Consume(TokenType::RPAREN, "Expect ')'.");
		return groupExpr;
	}

	Expr* Parser::ParseArrayExpr()
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

	Expr* Parser::ParseTableExpr()
	{
		Consume(TokenType::LBRACE, "Expect '{'.");

		std::unordered_map<Expr*, Expr*> elements;

		if (!IsMatchCurToken(TokenType::RBRACE))
		{
			Expr* key = ParseExpr();
			Consume(TokenType::COLON, "Expect ':' after table key.");
			Expr* value = ParseExpr();
			elements[key] = value;
			while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
			{
				Expr* key = ParseExpr();
				Consume(TokenType::COLON, "Expect ':' after table key.");
				Expr* value = ParseExpr();
				elements[key] = value;
			}
		}
		Consume(TokenType::RBRACE, "Expect '}' after table.");
		return new TableExpr(elements);
	}

	Expr* Parser::ParsePrefixExpr()
	{
		auto prefixExpr = new PrefixExpr();
		prefixExpr->op = GetCurTokenAndStepOnce().literal;
		prefixExpr->right = ParseExpr(Precedence::PREFIX);
		return prefixExpr;
	}

	Expr* Parser::ParseRefExpr()
	{
		Consume(TokenType::AMPERSAND, "Expect 'ref' keyword.");
		return new RefExpr(ParseExpr(Precedence::PREFIX));
	}

	Expr* Parser::ParseNewExpr()
	{
		Consume(TokenType::NEW, "Expect 'new keyword'");
		return new NewExpr((IdentifierExpr*)ParseIdentifierExpr());
	}

	Expr* Parser::ParseInfixExpr(Expr* prefixExpr)
	{
		auto infixExpr = new InfixExpr();
		infixExpr->left = prefixExpr;

		Precedence opPrece = GetCurTokenPrecedence();

		infixExpr->op = GetCurTokenAndStepOnce().literal;
		infixExpr->right = ParseExpr(opPrece);
		return infixExpr;
	}

	Expr* Parser::ParseConditionExpr(Expr* prefixExpr)
	{
		ConditionExpr* conditionExpr = new ConditionExpr();
		conditionExpr->condition = prefixExpr;

		Consume(TokenType::QUESTION, "Expect '?'.");

		conditionExpr->trueBranch = ParseExpr(Precedence::CONDITION);
		Consume(TokenType::COLON, "Expect ':' in condition expr");
		conditionExpr->falseBranch = ParseExpr(Precedence::CONDITION);

		return conditionExpr;
	}

	Expr* Parser::ParseIndexExpr(Expr* prefixExpr)
	{
		Consume(TokenType::LBRACKET, "Expect '['.");
		auto indexExpr = new IndexExpr();
		indexExpr->ds = prefixExpr;
		indexExpr->index = ParseExpr(Precedence::INFIX);
		Consume(TokenType::RBRACKET, "Expect ']'.");
		return indexExpr;
	}

	Expr* Parser::ParseFunctionCallExpr(Expr* prefixExpr)
	{
		auto funcCallExpr = new FunctionCallExpr();

		funcCallExpr->name = ((IdentifierExpr*)prefixExpr)->literal;
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

	Expr* Parser::ParseClassCallExpr(Expr* prefixExpr)
	{
		Consume(TokenType::DOT, "Expect '.'.");
		auto classCallExpr = new ClassCallExpr();
		classCallExpr->callee = prefixExpr;
		classCallExpr->callMember = ParseExpr(Precedence::INFIX);
		return classCallExpr;
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

	Associativity Parser::GetCurTokenAssociativity()
	{
		if (m_Associativity.find(GetCurTokenPrecedence()) != m_Associativity.end())
			return m_Associativity[GetCurTokenPrecedence()];
		return Associativity::LEFT2RIGHT;
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
		Assert("[line " + std::to_string(GetCurToken().line) + "]:" + std::string(errMsg));
		//avoid warning
		return Token(TokenType::END, "", 0);
	}

	Token Parser::Consume(const std::vector<TokenType>& types, std::string_view errMsg)
	{
		for (const auto& type : types)
			if (IsMatchCurToken(type))
				return GetCurTokenAndStepOnce();
		Assert("[line " + std::to_string(GetCurToken().line) + "]:" + std::string(errMsg));
		//avoid warning
		return Token(TokenType::END, "", 0);
	}

	bool Parser::IsAtEnd()
	{
		return m_CurPos >= (int32_t)m_Tokens.size();
	}
}