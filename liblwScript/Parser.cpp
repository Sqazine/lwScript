#include "Parser.h"
namespace lws
{
	struct PrecedenceBinding
	{
		TokenType type;
		Precedence precedence;
	};

	constexpr PrecedenceBinding precedenceTable[] = {
		{TOKEN_EQUAL, Precedence::ASSIGN},
		{TOKEN_PLUS_EQUAL, Precedence::ASSIGN},
		{TOKEN_MINUS_EQUAL, Precedence::ASSIGN},
		{TOKEN_ASTERISK_EQUAL, Precedence::ASSIGN},
		{TOKEN_SLASH_EQUAL, Precedence::ASSIGN},
		{TOKEN_PERCENT_EQUAL, Precedence::ASSIGN},
		{TOKEN_AMPERSAND_EQUAL, Precedence::ASSIGN},
		{TOKEN_VBAR_EQUAL, Precedence::ASSIGN},
		{TOKEN_CARET_EQUAL, Precedence::ASSIGN},
		{TOKEN_LESS_LESS_EQUAL, Precedence::ASSIGN},
		{TOKEN_GREATER_GREATER_EQUAL, Precedence::ASSIGN},
		{TOKEN_VBAR_VBAR, Precedence::OR},
		{TOKEN_AMPERSAND_AMPERSAND, Precedence::AND},
		{TOKEN_QUESTION, Precedence::CONDITION},
		{TOKEN_VBAR, Precedence::BIT_OR},
		{TOKEN_CARET, Precedence::BIT_XOR},
		{TOKEN_AMPERSAND, Precedence::BIT_AND},
		{TOKEN_EQUAL_EQUAL, Precedence::EQUAL},
		{TOKEN_BANG_EQUAL, Precedence::EQUAL},
		{TOKEN_LESS, Precedence::COMPARE},
		{TOKEN_LESS_EQUAL, Precedence::COMPARE},
		{TOKEN_GREATER, Precedence::COMPARE},
		{TOKEN_GREATER_EQUAL, Precedence::COMPARE},
		{TOKEN_LESS_LESS, Precedence::BIT_SHIFT},
		{TOKEN_GREATER_GREATER, Precedence::BIT_SHIFT},
		{TOKEN_PLUS, Precedence::ADD_PLUS},
		{TOKEN_MINUS, Precedence::ADD_PLUS},
		{TOKEN_ASTERISK, Precedence::MUL_DIV_MOD},
		{TOKEN_SLASH, Precedence::MUL_DIV_MOD},
		{TOKEN_PERCENT, Precedence::MUL_DIV_MOD},
		{TOKEN_LBRACKET, Precedence::INFIX},
		{TOKEN_LPAREN, Precedence::INFIX},
		{TOKEN_DOT, Precedence::INFIX},
	};

	struct AssociativityBinding
	{
		Precedence precedence;
		Associativity associativity;
	};

	constexpr AssociativityBinding associativityTable[] =
		{
			{Precedence::LOWEST, Associativity::L2R},
			{Precedence::ASSIGN, Associativity::R2L},
			{Precedence::CONDITION, Associativity::L2R},
			{Precedence::OR, Associativity::L2R},
			{Precedence::AND, Associativity::L2R},
			{Precedence::BIT_OR, Associativity::L2R},
			{Precedence::BIT_XOR, Associativity::L2R},
			{Precedence::BIT_AND, Associativity::L2R},
			{Precedence::EQUAL, Associativity::L2R},
			{Precedence::COMPARE, Associativity::L2R},
			{Precedence::BIT_SHIFT, Associativity::L2R},
			{Precedence::ADD_PLUS, Associativity::L2R},
			{Precedence::MUL_DIV_MOD, Associativity::L2R},
			{Precedence::PREFIX, Associativity::R2L},
			{Precedence::INFIX, Associativity::L2R},
	};

	std::unordered_map<TokenType, PrefixFn> Parser::mPrefixFunctions =
		{
			{TOKEN_IDENTIFIER, &Parser::ParseIdentifierExpr},
			{TOKEN_NUMBER, &Parser::ParseNumExpr},
			{TOKEN_STRING, &Parser::ParseStrExpr},
			{TOKEN_NULL, &Parser::ParseNullExpr},
			{TOKEN_TRUE, &Parser::ParseTrueExpr},
			{TOKEN_FALSE, &Parser::ParseFalseExpr},
			{TOKEN_MINUS, &Parser::ParsePrefixExpr},
			{TOKEN_TILDE, &Parser::ParsePrefixExpr},
			{TOKEN_BANG, &Parser::ParsePrefixExpr},
			{TOKEN_LPAREN, &Parser::ParseGroupExpr},
			{TOKEN_LBRACKET, &Parser::ParseArrayExpr},
			{TOKEN_LBRACE, &Parser::ParseTableExpr},
			{TOKEN_AMPERSAND, &Parser::ParseRefExpr},
			{TOKEN_FUNCTION, &Parser::ParseLambdaExpr},
			{TOKEN_CONTINUE, &Parser::ParseContinueExpr},
			{TOKEN_BREAK, &Parser::ParseBreakExpr},
	};

	std::unordered_map<TokenType, InfixFn> Parser::mInfixFunctions =
		{
			{TOKEN_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_PLUS_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_MINUS_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_ASTERISK_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_SLASH_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_PERCENT_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_AMPERSAND_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_VBAR_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_CARET_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_LESS_LESS_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_GREATER_GREATER_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_QUESTION, &Parser::ParseConditionExpr},
			{TOKEN_VBAR_VBAR, &Parser::ParseInfixExpr},
			{TOKEN_AMPERSAND_AMPERSAND, &Parser::ParseInfixExpr},
			{TOKEN_VBAR, &Parser::ParseInfixExpr},
			{TOKEN_CARET, &Parser::ParseInfixExpr},
			{TOKEN_AMPERSAND, &Parser::ParseInfixExpr},
			{TOKEN_LESS_LESS, &Parser::ParseInfixExpr},
			{TOKEN_GREATER_GREATER, &Parser::ParseInfixExpr},
			{TOKEN_EQUAL_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_BANG_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_LESS, &Parser::ParseInfixExpr},
			{TOKEN_LESS_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_GREATER, &Parser::ParseInfixExpr},
			{TOKEN_GREATER_EQUAL, &Parser::ParseInfixExpr},
			{TOKEN_PLUS, &Parser::ParseInfixExpr},
			{TOKEN_MINUS, &Parser::ParseInfixExpr},
			{TOKEN_ASTERISK, &Parser::ParseInfixExpr},
			{TOKEN_SLASH, &Parser::ParseInfixExpr},
			{TOKEN_PERCENT, &Parser::ParseInfixExpr},
			{TOKEN_LPAREN, &Parser::ParseFunctionCallExpr},
			{TOKEN_LBRACKET, &Parser::ParseIndexExpr},
			{TOKEN_DOT, &Parser::ParseFieldCallExpr},
	};

	Parser::Parser()
		: mStmts(nullptr)
	{
	}
	Parser::~Parser()
	{
		if (mStmts != nullptr)
		{
			delete mStmts;
			mStmts = nullptr;
		}

		std::unordered_map<TokenType, PrefixFn>().swap(mPrefixFunctions);
		std::unordered_map<TokenType, InfixFn>().swap(mInfixFunctions);
	}

	Stmt *Parser::Parse(const std::vector<Token> &tokens)
	{
		ResetStatus();
		mTokens = tokens;

		return ParseAstStmts();
	}

	void Parser::ResetStatus()
	{
		mCurPos = 0;

		if (mStmts != nullptr)
		{
			delete mStmts;
			mStmts = nullptr;
		}
		mStmts = new AstStmts();
	}

	Stmt *Parser::ParseAstStmts()
	{
		while (!IsMatchCurToken(TOKEN_END))
			mStmts->stmts.emplace_back(ParseStmt());
		return mStmts;
	}

	Stmt *Parser::ParseStmt()
	{
		if (IsMatchCurToken(TOKEN_LET))
			return ParseLetStmt();
		else if (IsMatchCurToken(TOKEN_CONST))
			return ParseConstStmt();
		else if (IsMatchCurToken(TOKEN_RETURN))
			return ParseReturnStmt();
		else if (IsMatchCurToken(TOKEN_IF))
			return ParseIfStmt();
		else if (IsMatchCurToken(TOKEN_LBRACE))
			return ParseScopeStmt();
		else if (IsMatchCurToken(TOKEN_WHILE))
			return ParseWhileStmt();
		else if (IsMatchCurToken(TOKEN_FOR))
			return ParseForStmt();
		else if (IsMatchCurToken(TOKEN_FUNCTION))
			return ParseFunctionStmt();
		else if (IsMatchCurToken(TOKEN_FIELD))
			return ParseFieldStmt();
		else
			return ParseExprStmt();
	}

	Stmt *Parser::ParseExprStmt()
	{
		auto exprStmt = new ExprStmt(ParseExpr());
		Consume(TOKEN_SEMICOLON, "Expect ';' after expr stmt.");
		return exprStmt;
	}

	Stmt *Parser::ParseLetStmt()
	{
		Consume(TOKEN_LET, "Expect 'let' key word");

		std::unordered_map<IdentifierExpr *, Expr *> variables;

		auto name = (IdentifierExpr *)ParseIdentifierExpr();
		Expr *value = new NullExpr();
		if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
			value = ParseExpr();
		variables[name] = value;

		while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
		{
			auto name = (IdentifierExpr *)ParseIdentifierExpr();
			Expr *value = new NullExpr();
			if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
				value = ParseExpr();
			variables[name] = value;
		}

		Consume(TOKEN_SEMICOLON, "Expect ';' after let stmt.");

		return new LetStmt(variables);
	}

	Stmt *Parser::ParseConstStmt()
	{
		Consume(TOKEN_CONST, "Expect 'const' key word");

		std::unordered_map<IdentifierExpr *, Expr *> consts;

		auto name = (IdentifierExpr *)ParseIdentifierExpr();
		Expr *value = new NullExpr();
		if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
			value = ParseExpr();
		consts[name] = value;

		while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
		{
			auto name = (IdentifierExpr *)ParseIdentifierExpr();
			Expr *value = new NullExpr();
			if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
				value = ParseExpr();
			consts[name] = value;
		}

		Consume(TOKEN_SEMICOLON, "Expect ';' after const stmt.");

		return new ConstStmt(consts);
	}

	Stmt *Parser::ParseReturnStmt()
	{
		Consume(TOKEN_RETURN, "Expect 'return' key word.");

		auto returnStmt = new ReturnStmt();

		if (!IsMatchCurToken(TOKEN_SEMICOLON))
			returnStmt->expr = ParseExpr();

		Consume(TOKEN_SEMICOLON, "Expect ';' after return stmt");
		return returnStmt;
	}

	Stmt *Parser::ParseIfStmt()
	{
		Consume(TOKEN_IF, "Expect 'if' key word.");
		Consume(TOKEN_LPAREN, "Expect '(' after 'if'.");

		auto ifStmt = new IfStmt();

		ifStmt->condition = ParseExpr();

		Consume(TOKEN_RPAREN, "Expect ')' after if condition");

		ifStmt->thenBranch = ParseStmt();

		if (IsMatchCurTokenAndStepOnce(TOKEN_ELSE))
			ifStmt->elseBranch = ParseStmt();

		return ifStmt;
	}

	Stmt *Parser::ParseScopeStmt()
	{
		Consume(TOKEN_LBRACE, "Expect '{'.");
		auto scopeStmt = new ScopeStmt();
		while (!IsMatchCurToken(TOKEN_RBRACE))
			scopeStmt->stmts.emplace_back(ParseStmt());
		Consume(TOKEN_RBRACE, "Expect '}'.");
		return scopeStmt;
	}

	Stmt *Parser::ParseWhileStmt()
	{
		Consume(TOKEN_WHILE, "Expect 'while' keyword.");
		Consume(TOKEN_LPAREN, "Expect '(' after 'while'.");

		auto whileStmt = new WhileStmt();

		whileStmt->condition = ParseExpr();

		Consume(TOKEN_RPAREN, "Expect ')' after while stmt's condition.");

		whileStmt->body = ParseStmt();

		return whileStmt;
	}

	Stmt *Parser::ParseForStmt()
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
		Consume(TOKEN_FOR, "Expect 'for' keyword.");
		Consume(TOKEN_LPAREN, "Expect '(' after 'for'.");

		//initializer
		auto scopeStmt = new ScopeStmt();
		if (IsMatchCurToken(TOKEN_LET))
			scopeStmt->stmts.emplace_back(ParseLetStmt());
		else
		{
			if (!IsMatchCurToken(TOKEN_SEMICOLON))
			{
				scopeStmt->stmts.emplace_back(new ExprStmt(ParseExpr()));
				while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
					scopeStmt->stmts.emplace_back(new ExprStmt(ParseExpr()));
			}
			Consume(TOKEN_SEMICOLON, "Expect ';' after for stmt's initializer stmt");
		}

		Expr *condition = nullptr;
		if (!IsMatchCurToken(TOKEN_SEMICOLON))
			condition = ParseExpr();
		Consume(TOKEN_SEMICOLON, "Expect ';' after for stmt's condition expr.");

		std::vector<Expr *> increment;
		if (!IsMatchCurToken(TOKEN_RPAREN))
		{
			increment.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
				increment.emplace_back(ParseExpr());
		}
		Consume(TOKEN_RPAREN, "Expect ')' after for stmt's increment expr(s)");

		std::vector<Stmt *> whileBodyStmts;
		whileBodyStmts.emplace_back(ParseStmt());
		for (const auto expr : increment)
			whileBodyStmts.emplace_back(new ExprStmt(expr));

		auto whileStmt = new WhileStmt();
		whileStmt->condition = condition;
		whileStmt->body = new ScopeStmt(whileBodyStmts);

		scopeStmt->stmts.emplace_back(whileStmt);

		return scopeStmt;
	}

	Stmt *Parser::ParseFunctionStmt()
	{
		Consume(TOKEN_FUNCTION, "Expect 'function' keyword");

		auto funcStmt = new FunctionStmt();

		funcStmt->name = (IdentifierExpr *)ParseIdentifierExpr();

		Consume(TOKEN_LPAREN, "Expect '(' after 'function' keyword");

		if (!IsMatchCurToken(TOKEN_RPAREN)) //has parameter
		{
			IdentifierExpr *idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
			funcStmt->parameters.emplace_back(idenExpr);
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
			{
				idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
				funcStmt->parameters.emplace_back(idenExpr);
			}
		}
		Consume(TOKEN_RPAREN, "Expect ')' after function stmt's '('");

		funcStmt->name->literal += std::to_string(funcStmt->parameters.size());

		funcStmt->body = (ScopeStmt *)ParseScopeStmt();

		return funcStmt;
	}

	Expr *Parser::ParseLambdaExpr()
	{
		Consume(TOKEN_FUNCTION, "Expect 'fn' keyword");

		auto lambdaExpr = new LambdaExpr();

		Consume(TOKEN_LPAREN, "Expect '(' after 'fn' keyword");

		if (!IsMatchCurToken(TOKEN_RPAREN)) //has parameter
		{
			IdentifierExpr *idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
			lambdaExpr->parameters.emplace_back(idenExpr);
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
			{
				idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
				lambdaExpr->parameters.emplace_back(idenExpr);
			}
		}
		Consume(TOKEN_RPAREN, "Expect ')' after lambda expr's '('");

		lambdaExpr->body = (ScopeStmt *)ParseScopeStmt();

		return lambdaExpr;
	}

	Expr *Parser::ParseBreakExpr()
	{
		Consume(TOKEN_BREAK, "Expect 'break' keyword");
		return new BreakExpr();
	}

	Expr *Parser::ParseContinueExpr()
	{
		Consume(TOKEN_CONTINUE, "Expect 'continue' keyword");
		return new ContinueExpr();
	}

	Stmt *Parser::ParseFieldStmt()
	{
		Consume(TOKEN_FIELD, "Expect 'field' keyword");

		auto fieldStmt = new FieldStmt();
		fieldStmt->name = ((IdentifierExpr *)ParseIdentifierExpr())->literal;

		if (IsMatchCurTokenAndStepOnce(TOKEN_COLON))
		{
			fieldStmt->containedFields.emplace_back((IdentifierExpr *)ParseIdentifierExpr());
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
				fieldStmt->containedFields.emplace_back((IdentifierExpr *)ParseIdentifierExpr());
		}

		Consume(TOKEN_LBRACE, "Expect '{' after field name or contained field name");

		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			if (IsMatchCurToken(TOKEN_LET))
				fieldStmt->letStmts.emplace_back((LetStmt *)ParseLetStmt());
			else if (IsMatchCurToken(TOKEN_FUNCTION))
				fieldStmt->fnStmts.emplace_back((FunctionStmt *)ParseFunctionStmt());
			else
				Consume({TOKEN_LET, TOKEN_FUNCTION}, "UnExpect identifier '" + GetCurToken().literal + "'.");
		}

		Consume(TOKEN_RBRACE, "Expect '}' after field stmt's '{'");

		return fieldStmt;
	}

	Expr *Parser::ParseExpr(Precedence precedence)
	{
		if (mPrefixFunctions.find(GetCurToken().type) == mPrefixFunctions.end())
		{
			std::cout << "no prefix definition for:" << GetCurTokenAndStepOnce().literal << std::endl;
			return new NullExpr();
		}
		auto prefixFn = mPrefixFunctions[GetCurToken().type];

		auto leftExpr = (this->*prefixFn)();

		while (!IsMatchCurToken(TOKEN_SEMICOLON) && (GetCurTokenAssociativity() == Associativity::L2R ? precedence < GetCurTokenPrecedence() : precedence <= GetCurTokenPrecedence()))
		{
			if (mInfixFunctions.find(GetCurToken().type) == mInfixFunctions.end())
				return leftExpr;

			auto infixFn = mInfixFunctions[GetCurToken().type];

			leftExpr = (this->*infixFn)(leftExpr);
		}

		return leftExpr;
	}

	Expr *Parser::ParseIdentifierExpr()
	{
		return new IdentifierExpr(Consume(TOKEN_IDENTIFIER, "Unexpect Identifier'" + GetCurToken().literal + "'.").literal);
	}

	Expr *Parser::ParseNumExpr()
	{
		std::string numLiteral = Consume(TOKEN_NUMBER, "Expexct a number literal.").literal;
		if (numLiteral.find('.') != std::string::npos)
			return new RealNumExpr(std::stod(numLiteral));
		else
			return new IntNumExpr(std::stoll(numLiteral));
	}

	Expr *Parser::ParseStrExpr()
	{
		return new StrExpr(Consume(TOKEN_STRING, "Expect a string literal.").literal);
	}

	Expr *Parser::ParseNullExpr()
	{
		Consume(TOKEN_NULL, "Expect 'null' keyword");
		return new NullExpr();
	}
	Expr *Parser::ParseTrueExpr()
	{
		Consume(TOKEN_TRUE, "Expect 'true' keyword");
		return new BoolExpr(true);
	}
	Expr *Parser::ParseFalseExpr()
	{
		Consume(TOKEN_FALSE, "Expect 'false' keyword");
		return new BoolExpr(false);
	}

	Expr *Parser::ParseGroupExpr()
	{
		Consume(TOKEN_LPAREN, "Expect '('.");
		auto groupExpr = new GroupExpr(ParseExpr(Precedence::INFIX));
		Consume(TOKEN_RPAREN, "Expect ')'.");
		return groupExpr;
	}

	Expr *Parser::ParseArrayExpr()
	{
		Consume(TOKEN_LBRACKET, "Expect '['.");

		auto arrayExpr = new ArrayExpr();
		if (!IsMatchCurToken(TOKEN_RBRACKET))
		{
			//first element
			arrayExpr->elements.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
				arrayExpr->elements.emplace_back(ParseExpr());
		}

		Consume(TOKEN_RBRACKET, "Expect ']'.");

		return arrayExpr;
	}

	Expr *Parser::ParseTableExpr()
	{
		Consume(TOKEN_LBRACE, "Expect '{'.");

		std::unordered_map<Expr *, Expr *> elements;

		if (!IsMatchCurToken(TOKEN_RBRACE))
		{
			Expr *key = ParseExpr();
			Consume(TOKEN_COLON, "Expect ':' after table key.");
			Expr *value = ParseExpr();
			elements[key] = value;
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
			{
				Expr *key = ParseExpr();
				Consume(TOKEN_COLON, "Expect ':' after table key.");
				Expr *value = ParseExpr();
				elements[key] = value;
			}
		}
		Consume(TOKEN_RBRACE, "Expect '}' after table.");
		return new TableExpr(elements);
	}

	Expr *Parser::ParsePrefixExpr()
	{
		auto prefixExpr = new PrefixExpr();
		prefixExpr->op = GetCurTokenAndStepOnce().literal;
		prefixExpr->right = ParseExpr(Precedence::PREFIX);
		return prefixExpr;
	}

	Expr *Parser::ParseRefExpr()
	{
		Consume(TOKEN_AMPERSAND, "Expect '&'.");
		return new RefExpr(ParseExpr(Precedence::PREFIX));
	}

	Expr *Parser::ParseInfixExpr(Expr *prefixExpr)
	{
		auto infixExpr = new InfixExpr();
		infixExpr->left = prefixExpr;

		Precedence opPrece = GetCurTokenPrecedence();

		infixExpr->op = GetCurTokenAndStepOnce().literal;
		infixExpr->right = ParseExpr(opPrece);
		return infixExpr;
	}

	Expr *Parser::ParseConditionExpr(Expr *prefixExpr)
	{
		ConditionExpr *conditionExpr = new ConditionExpr();
		conditionExpr->condition = prefixExpr;

		Consume(TOKEN_QUESTION, "Expect '?'.");

		conditionExpr->trueBranch = ParseExpr(Precedence::CONDITION);
		Consume(TOKEN_COLON, "Expect ':' in condition expr");
		conditionExpr->falseBranch = ParseExpr(Precedence::CONDITION);

		return conditionExpr;
	}

	Expr *Parser::ParseIndexExpr(Expr *prefixExpr)
	{
		Consume(TOKEN_LBRACKET, "Expect '['.");
		auto indexExpr = new IndexExpr();
		indexExpr->ds = prefixExpr;
		indexExpr->index = ParseExpr();
		Consume(TOKEN_RBRACKET, "Expect ']'.");
		return indexExpr;
	}

	Expr *Parser::ParseFunctionCallExpr(Expr *prefixExpr)
	{
		auto funcCallExpr = new FunctionCallExpr();

		funcCallExpr->name = prefixExpr;
		Consume(TOKEN_LPAREN, "Expect '('.");
		if (!IsMatchCurToken(TOKEN_RPAREN)) //has arguments
		{
			funcCallExpr->arguments.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
				funcCallExpr->arguments.emplace_back(ParseExpr());
		}
		Consume(TOKEN_RPAREN, "Expect ')'.");

		return funcCallExpr;
	}

	Expr *Parser::ParseFieldCallExpr(Expr *prefixExpr)
	{
		Consume(TOKEN_DOT, "Expect '.'.");
		auto fieldCallExpr = new FieldCallExpr();
		fieldCallExpr->callee = prefixExpr;
		fieldCallExpr->callMember = ParseExpr(Precedence::INFIX);
		return fieldCallExpr;
	}

	Token Parser::GetCurToken()
	{
		if (!IsAtEnd())
			return mTokens[mCurPos];
		return mTokens.back();
	}
	Token Parser::GetCurTokenAndStepOnce()
	{
		if (!IsAtEnd())
			return mTokens[mCurPos++];
		return mTokens.back();
	}

	Precedence Parser::GetCurTokenPrecedence()
	{
		for (const auto &precedenceBinding : precedenceTable)
			if (precedenceBinding.type == GetCurToken().type)
				return precedenceBinding.precedence;
		return Precedence::LOWEST;
	}

	Associativity Parser::GetCurTokenAssociativity()
	{
		for (const auto &associativityBinding : associativityTable)
			if (associativityBinding.precedence == GetCurTokenPrecedence())
				return associativityBinding.associativity;
		return Associativity::L2R;
	}

	Token Parser::GetNextToken()
	{
		if (mCurPos + 1 < (int32_t)mTokens.size())
			return mTokens[mCurPos + 1];
		return mTokens.back();
	}
	Token Parser::GetNextTokenAndStepOnce()
	{
		if (mCurPos + 1 < (int32_t)mTokens.size())
			return mTokens[++mCurPos];
		return mTokens.back();
	}

	Precedence Parser::GetNextTokenPrecedence()
	{
		for (const auto &precedenceBinding : precedenceTable)
			if (precedenceBinding.type == GetNextToken().type)
				return precedenceBinding.precedence;
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
			mCurPos++;
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
			mCurPos++;
			return true;
		}
		return false;
	}

	Token Parser::Consume(TokenType type, std::string_view errMsg)
	{
		if (IsMatchCurToken(type))
			return GetCurTokenAndStepOnce();
		Assert("[line " + std::to_string(GetCurToken().line) + "]:" + std::string(errMsg));
		//avoid C++ compiler warning
		return Token(TOKEN_END, "", 0);
	}

	Token Parser::Consume(const std::vector<TokenType> &types, std::string_view errMsg)
	{
		for (const auto &type : types)
			if (IsMatchCurToken(type))
				return GetCurTokenAndStepOnce();
		Assert("[line " + std::to_string(GetCurToken().line) + "]:" + std::string(errMsg));
		//avoid C++ compiler warning
		return Token(TOKEN_END, "", 0);
	}

	bool Parser::IsAtEnd()
	{
		return mCurPos >= (int32_t)mTokens.size();
	}
}