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
		{TOKEN_PLUS_PLUS, Precedence::POSTFIX},
		{TOKEN_MINUS_MINUS, Precedence::POSTFIX},
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
			{Precedence::POSTFIX, Associativity::L2R}};

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
			{TOKEN_PLUS_PLUS, &Parser::ParsePrefixExpr},
			{TOKEN_MINUS_MINUS, &Parser::ParsePrefixExpr},
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

	std::unordered_map<TokenType, PostfixFn> Parser::mPostfixFunctions =
		{
			{TOKEN_PLUS_PLUS, &Parser::ParsePostfixExpr},
			{TOKEN_MINUS_MINUS, &Parser::ParsePostfixExpr},
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
		else if (IsMatchCurToken(TOKEN_BREAK))
			return ParseBreakStmt();
		else if (IsMatchCurToken(TOKEN_CONTINUE))
			return ParseContinueStmt();
		else if (IsMatchCurToken(TOKEN_SWITCH))
			return ParseSwitchStmt();
		else if (IsMatchCurToken(TOKEN_MATCH))
			return ParseMatchStmt();
		else if (IsMatchCurToken(TOKEN_ENUM))
			return ParseEnumStmt();
		else if (IsMatchCurToken(TOKEN_FUNCTION))
			return ParseFunctionStmt();
		else if (IsMatchCurToken(TOKEN_FIELD))
			return ParseFieldStmt();
		else
			return ParseExprStmt();
	}

	Stmt *Parser::ParseExprStmt()
	{
		auto exprStmt = new ExprStmt();
		exprStmt->line = GetCurToken().line;
		exprStmt->column = GetCurToken().column;

		exprStmt->expr = ParseExpr();

		Consume(TOKEN_SEMICOLON, L"Expect ';' after expr stmt.");
		return exprStmt;
	}

	Stmt *Parser::ParseLetStmt()
	{
		auto letStmt = new LetStmt();
		letStmt->line = GetCurToken().line;
		letStmt->column = GetCurToken().column;

		Consume(TOKEN_LET, L"Expect 'let' key word");

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

		Consume(TOKEN_SEMICOLON, L"Expect ';' after let stmt.");

		letStmt->variables = variables;

		return letStmt;
	}

	Stmt *Parser::ParseConstStmt()
	{
		auto constStmt = new ConstStmt();
		constStmt->line = GetCurToken().line;
		constStmt->column = GetCurToken().column;

		Consume(TOKEN_CONST, L"Expect 'const' key word");

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

		Consume(TOKEN_SEMICOLON, L"Expect ';' after const stmt.");

		constStmt->consts = consts;

		return constStmt;
	}

	Stmt *Parser::ParseReturnStmt()
	{
		auto returnStmt = new ReturnStmt();
		returnStmt->line = GetCurToken().line;
		returnStmt->column = GetCurToken().column;
		Consume(TOKEN_RETURN, L"Expect 'return' key word.");

		if (!IsMatchCurToken(TOKEN_SEMICOLON))
			returnStmt->expr = ParseExpr();

		Consume(TOKEN_SEMICOLON, L"Expect ';' after return stmt");
		return returnStmt;
	}

	Stmt *Parser::ParseIfStmt()
	{
		auto ifStmt = new IfStmt();
		ifStmt->column = GetCurToken().column;
		ifStmt->line = GetCurToken().line;

		Consume(TOKEN_IF, L"Expect 'if' key word.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'if'.");

		ifStmt->condition = ParseExpr();

		Consume(TOKEN_RPAREN, L"Expect ')' after if condition");

		ifStmt->thenBranch = ParseStmt();

		if (IsMatchCurTokenAndStepOnce(TOKEN_ELSE))
			ifStmt->elseBranch = ParseStmt();

		return ifStmt;
	}

	Stmt *Parser::ParseScopeStmt()
	{
		auto scopeStmt = new ScopeStmt();
		scopeStmt->line = GetCurToken().line;
		scopeStmt->column = GetCurToken().column;

		Consume(TOKEN_LBRACE, L"Expect '{'.");
		while (!IsMatchCurToken(TOKEN_RBRACE))
			scopeStmt->stmts.emplace_back(ParseStmt());
		Consume(TOKEN_RBRACE, L"Expect '}'.");
		return scopeStmt;
	}

	Stmt *Parser::ParseWhileStmt()
	{
		auto whileStmt = new WhileStmt();
		whileStmt->line = GetCurToken().line;
		whileStmt->column = GetCurToken().column;
		Consume(TOKEN_WHILE, L"Expect 'while' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'while'.");

		whileStmt->condition = ParseExpr();

		Consume(TOKEN_RPAREN, L"Expect ')' after while stmt's condition.");

		if (IsMatchCurToken(TOKEN_LBRACE)) // scope stmt:while(a<10){a=a+1;}
			whileStmt->body = (ScopeStmt *)ParseScopeStmt();
		else // single stmt:while(a<10) a=a+1;
		{
			auto scopeStmt = new ScopeStmt();
			scopeStmt->stmts.emplace_back(ParseStmt());
			whileStmt->body = scopeStmt;
		}

		return whileStmt;
	}

	Stmt *Parser::ParseForStmt()
	{
		// for(let i=0,j=0;i<10&&j<10;i=i+1,j=j+1)
		//{
		//	...
		// }
		//  |
		//  |
		//  v
		//{
		//		let i=0,j=0;
		//		while(i<10&&j<10)
		//		{
		//			{
		//				...
		//			}
		//			increment part:
		//			{
		//				i=i+1;
		//				j=j+1;
		//			}
		//		}
		// }
		auto scopeStmt = new ScopeStmt();
		scopeStmt->line = GetCurToken().line;
		scopeStmt->column = GetCurToken().column;

		Consume(TOKEN_FOR, L"Expect 'for' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'for'.");

		// initializer
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
			Consume(TOKEN_SEMICOLON, L"Expect ';' after for stmt's initializer stmt");
		}

		Expr *condition = nullptr;
		if (!IsMatchCurToken(TOKEN_SEMICOLON))
			condition = ParseExpr();
		Consume(TOKEN_SEMICOLON, L"Expect ';' after for stmt's condition expr.");

		std::vector<Expr *> increment;
		if (!IsMatchCurToken(TOKEN_RPAREN))
		{
			increment.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
				increment.emplace_back(ParseExpr());
		}
		Consume(TOKEN_RPAREN, L"Expect ')' after for stmt's increment expr(s)");

		std::vector<Stmt *> whileBodyStmts;
		if (IsMatchCurToken(TOKEN_LBRACE)) // scope stmt for 'for' stmt:like for(a=0;a<10;a=a+1){println("{}",a);}
			whileBodyStmts.emplace_back(ParseScopeStmt());
		else // single stmt for 'for' stmt:like for(a=0;a<10;a=a+1) println("{}",a);
		{
			auto scopeStmt = new ScopeStmt();
			scopeStmt->stmts.emplace_back(ParseStmt());
			whileBodyStmts.emplace_back(scopeStmt);
		}

		auto whileStmt = new WhileStmt();
		whileStmt->condition = condition;
		whileStmt->body = new ScopeStmt(whileBodyStmts);

		std::vector<Stmt *> incrementStmts;
		for (const auto expr : increment)
			incrementStmts.emplace_back(new ExprStmt(expr));
		whileStmt->increment = new ScopeStmt(incrementStmts);

		scopeStmt->stmts.emplace_back(whileStmt);

		return scopeStmt;
	}

	Stmt *Parser::ParseBreakStmt()
	{
		auto breakStmt = new BreakStmt();
		breakStmt->line = GetCurToken().line;
		breakStmt->column = GetCurToken().column;
		Consume(TOKEN_BREAK, L"Expect 'break' keyword.");
		Consume(TOKEN_SEMICOLON, L"Expect ';' after 'break' keyword.");
		return breakStmt;
	}

	Stmt *Parser::ParseContinueStmt()
	{
		auto continueStmt = new ContinueStmt();
		continueStmt->line = GetCurToken().line;
		continueStmt->column = GetCurToken().column;

		Consume(TOKEN_CONTINUE, L"Expect 'continue' keyword");
		Consume(TOKEN_SEMICOLON, L"Expect ';' after 'continue' keyword.");

		return continueStmt;
	}

	Stmt *Parser::ParseSwitchStmt()
	{
		auto ifStmt = new IfStmt();
		ifStmt->line = GetCurToken().line;
		ifStmt->column = GetCurToken().column;

		Consume(TOKEN_SWITCH, L"Expect 'switch' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'switch' keyword.");
		auto switchExpr = ParseIdentifierExpr();
		switchExpr->column = GetCurToken().column;
		switchExpr->line = GetCurToken().line;
		Consume(TOKEN_RPAREN, L"Expect ')' after switch's expression.");
		Consume(TOKEN_LBRACE, L"Expect '{' after 'switch' keyword.");

		struct CaseItem
		{
			Expr *conditionExpr = nullptr;
			ScopeStmt *caseExecuteOpCodeScope = nullptr;

			bool IsValid()
			{
				return conditionExpr && caseExecuteOpCodeScope;
			}
		};
		std::vector<CaseItem> caseItems;
		ScopeStmt *defaultScopeStmt = nullptr;
		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			if (IsMatchCurTokenAndStepOnce(TOKEN_CASE)) //the first case
			{
				CaseItem item;

				auto valueCompareExpr = ParseExpr();
				Consume(TOKEN_COLON, L"Expect ':' after case's condition expr.");

				item.conditionExpr = new InfixExpr(L"==", switchExpr, valueCompareExpr);
				item.conditionExpr->line = GetCurToken().line;
				item.conditionExpr->column = GetCurToken().column;

				item.caseExecuteOpCodeScope = new ScopeStmt();
				item.caseExecuteOpCodeScope->line = GetCurToken().line;
				item.caseExecuteOpCodeScope->column = GetCurToken().column;

				if (IsMatchCurTokenAndStepOnce(TOKEN_LBRACE))
				{
					while (!IsMatchCurToken(TOKEN_RBRACE))
						item.caseExecuteOpCodeScope->stmts.emplace_back(ParseStmt());
					Consume(TOKEN_RBRACE, L"Expect '}' at the end of case block while has multiple statements.");
				}
				else
					item.caseExecuteOpCodeScope->stmts.emplace_back(ParseStmt());

				caseItems.emplace_back(item);
			}

			if (IsMatchCurTokenAndStepOnce(TOKEN_DEFAULT))
			{
				Consume(TOKEN_COLON, L"Expect ':' after case's condition expr.");
				defaultScopeStmt = new ScopeStmt();
				defaultScopeStmt->line = GetCurToken().line;
				defaultScopeStmt->column = GetCurToken().column;
				if (IsMatchCurTokenAndStepOnce(TOKEN_LBRACE))
				{
					while (!IsMatchCurToken(TOKEN_RBRACE))
						defaultScopeStmt->stmts.emplace_back(ParseStmt());
					Consume(TOKEN_RBRACE, L"Expect '}' at the end of default block while has multiple statement");
				}
				else
					defaultScopeStmt->stmts.emplace_back(ParseStmt());
			}
		}

		Consume(TOKEN_RBRACE, L"Expect '}' after switch stmt");

		if (caseItems.empty() && defaultScopeStmt != nullptr)
			return defaultScopeStmt;
		else
		{
			auto loopIfStmt = ifStmt;
			for (size_t i = 0; i < caseItems.size(); ++i)
			{
				loopIfStmt->condition = caseItems[i].conditionExpr;
				loopIfStmt->thenBranch = caseItems[i].caseExecuteOpCodeScope;
				if (i + 1 < caseItems.size())
				{
					loopIfStmt->elseBranch = new IfStmt();
					loopIfStmt = (IfStmt *)loopIfStmt->elseBranch;
				}
			}

			if (defaultScopeStmt)
				loopIfStmt->elseBranch = defaultScopeStmt;
		}
		return ifStmt;
	}

	Stmt *Parser::ParseMatchStmt()
	{
		auto ifStmt = new IfStmt();
		ifStmt->line = GetCurToken().line;
		ifStmt->column = GetCurToken().column;

		Consume(TOKEN_MATCH, L"Expect 'match' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'match' keyword.");
		auto matchExpr = ParseIdentifierExpr();
		matchExpr->column = GetCurToken().column;
		matchExpr->line = GetCurToken().line;
		Consume(TOKEN_RPAREN, L"Expect ')' after match's expression.");
		Consume(TOKEN_LBRACE, L"Expect '{' after 'match' keyword.");

		struct MatchItem
		{
			Expr *conditionExpr = nullptr;
			ScopeStmt *executeScope = nullptr;

			bool IsValid()
			{
				return conditionExpr && executeScope;
			}
		};
		std::vector<MatchItem> matchItems;
		ScopeStmt *defaultScopeStmt = nullptr;
		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			if (IsMatchCurTokenAndStepOnce(TOKEN_DEFAULT))
			{
				Consume(TOKEN_COLON, L"Expect ':' after default's condition expr.");
				defaultScopeStmt = new ScopeStmt();
				defaultScopeStmt->line = GetCurToken().line;
				defaultScopeStmt->column = GetCurToken().column;
				if (IsMatchCurTokenAndStepOnce(TOKEN_LBRACE))
				{
					while (!IsMatchCurToken(TOKEN_RBRACE))
						defaultScopeStmt->stmts.emplace_back(ParseStmt());
					Consume(TOKEN_RBRACE, L"Expect '}' at the end of default block while has multiple statement");
				}
				else
					defaultScopeStmt->stmts.emplace_back(ParseStmt());
			}
			else
			{
				MatchItem item;

				auto valueCompareExpr = ParseExpr();
				Consume(TOKEN_COLON, L"Expect ':' after match item's condition expr.");

				item.conditionExpr = new InfixExpr(L"==", matchExpr, valueCompareExpr);
				item.conditionExpr->line = GetCurToken().line;
				item.conditionExpr->column = GetCurToken().column;

				item.executeScope = new ScopeStmt();
				item.executeScope->line = GetCurToken().line;
				item.executeScope->column = GetCurToken().column;

				if (IsMatchCurTokenAndStepOnce(TOKEN_LBRACE))
				{
					while (!IsMatchCurToken(TOKEN_RBRACE))
						item.executeScope->stmts.emplace_back(ParseStmt());
					Consume(TOKEN_RBRACE, L"Expect '}' at the end of case block while has multiple statements.");
				}
				else
					item.executeScope->stmts.emplace_back(ParseStmt());

				matchItems.emplace_back(item);
			}
		}

		Consume(TOKEN_RBRACE, L"Expect '}' after match stmt");

		if (matchItems.empty() && defaultScopeStmt != nullptr)
			return defaultScopeStmt;
		else
		{
			auto loopIfStmt = ifStmt;
			for (size_t i = 0; i < matchItems.size(); ++i)
			{
				loopIfStmt->condition = matchItems[i].conditionExpr;
				loopIfStmt->thenBranch = matchItems[i].executeScope;
				if (i + 1 < matchItems.size())
				{
					loopIfStmt->elseBranch = new IfStmt();
					loopIfStmt = (IfStmt *)loopIfStmt->elseBranch;
				}
			}

			if (defaultScopeStmt)
				loopIfStmt->elseBranch = defaultScopeStmt;
		}
		return ifStmt;
	}

	Stmt *Parser::ParseEnumStmt()
	{
		auto enumStmt = new EnumStmt();
		enumStmt->line = GetCurToken().line;
		enumStmt->column = GetCurToken().column;
		Consume(TOKEN_ENUM, L"Expect 'enum' keyword.");
		enumStmt->enumName = (IdentifierExpr *)ParseIdentifierExpr();
		Consume(TOKEN_LBRACE, L"Expect '{' after 'enum' keyword.");

		std::unordered_map<IdentifierExpr *, Expr *> items;

		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			auto name = (IdentifierExpr *)ParseIdentifierExpr();
			Expr *value = new StrExpr(name->literal);
			if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
			{
				delete value;
				value = nullptr;
				value = ParseExpr();
			}

			items[name] = value;

			if (IsMatchCurToken(TOKEN_COMMA))
				Consume(TOKEN_COMMA, L"Expect ',' after enum item.");
		}

		Consume(TOKEN_RBRACE, L"Expect '}' at the end of the 'enum' stmt.");

		enumStmt->enumItems = items;

		return enumStmt;
	}

	Stmt *Parser::ParseFunctionStmt()
	{
		auto funcStmt = new FunctionStmt();
		funcStmt->line = GetCurToken().line;
		funcStmt->column = GetCurToken().column;

		Consume(TOKEN_FUNCTION, L"Expect 'function' keyword");

		funcStmt->name = (IdentifierExpr *)ParseIdentifierExpr();

		Consume(TOKEN_LPAREN, L"Expect '(' after 'function' keyword");

		if (!IsMatchCurToken(TOKEN_RPAREN)) // has parameter
		{
			IdentifierExpr *idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
			funcStmt->parameters.emplace_back(idenExpr);
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
			{
				idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
				funcStmt->parameters.emplace_back(idenExpr);
			}
		}
		Consume(TOKEN_RPAREN, L"Expect ')' after function stmt's '('");

		funcStmt->body = (ScopeStmt *)ParseScopeStmt();

		return funcStmt;
	}

	Expr *Parser::ParseLambdaExpr()
	{
		auto lambdaExpr = new LambdaExpr();
		lambdaExpr->line = GetCurToken().line;
		lambdaExpr->column = GetCurToken().column;

		Consume(TOKEN_FUNCTION, L"Expect 'fn' keyword");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'fn' keyword");

		if (!IsMatchCurToken(TOKEN_RPAREN)) // has parameter
		{
			IdentifierExpr *idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
			lambdaExpr->parameters.emplace_back(idenExpr);
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
			{
				idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
				lambdaExpr->parameters.emplace_back(idenExpr);
			}
		}
		Consume(TOKEN_RPAREN, L"Expect ')' after lambda expr's '('");

		lambdaExpr->body = (ScopeStmt *)ParseScopeStmt();

		return lambdaExpr;
	}

	Stmt *Parser::ParseFieldStmt()
	{
		auto fieldStmt = new FieldStmt();
		fieldStmt->line = GetCurToken().line;
		fieldStmt->column = GetCurToken().column;

		Consume(TOKEN_FIELD, L"Expect 'field' keyword");

		fieldStmt->name = ((IdentifierExpr *)ParseIdentifierExpr())->literal;

		if (IsMatchCurTokenAndStepOnce(TOKEN_COLON))
		{
			fieldStmt->containedFields.emplace_back((IdentifierExpr *)ParseIdentifierExpr());
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
				fieldStmt->containedFields.emplace_back((IdentifierExpr *)ParseIdentifierExpr());
		}

		Consume(TOKEN_LBRACE, L"Expect '{' after field name or contained field name");

		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			if (IsMatchCurToken(TOKEN_LET))
				fieldStmt->letStmts.emplace_back((LetStmt *)ParseLetStmt());
			else if (IsMatchCurToken(TOKEN_CONST))
				fieldStmt->constStmts.emplace_back((ConstStmt *)ParseConstStmt());
			else if (IsMatchCurToken(TOKEN_FUNCTION))
				fieldStmt->fnStmts.emplace_back((FunctionStmt *)ParseFunctionStmt());
			else
				Consume({TOKEN_LET, TOKEN_FUNCTION, TOKEN_CONST}, L"UnExpect identifier '" + GetCurToken().literal + L"'.");
		}

		Consume(TOKEN_RBRACE, L"Expect '}' after field stmt's '{'");

		return fieldStmt;
	}

	Expr *Parser::ParseExpr(Precedence precedence)
	{
		if (mPrefixFunctions.find(GetCurToken().type) == mPrefixFunctions.end())
		{
			std::wcout << L"no prefix definition for:" << GetCurTokenAndStepOnce().literal << std::endl;
			return new NullExpr();
		}
		auto prefixFn = mPrefixFunctions[GetCurToken().type];

		auto leftExpr = (this->*prefixFn)();

		while (!IsMatchCurToken(TOKEN_SEMICOLON) && (GetCurTokenAssociativity() == Associativity::L2R ? precedence < GetCurTokenPrecedence() : precedence <= GetCurTokenPrecedence()))
		{
			if (mPostfixFunctions.find(GetCurToken().type) != mPostfixFunctions.end())
			{
				auto postfixFn = mPostfixFunctions[GetCurToken().type];
				leftExpr = (this->*postfixFn)(leftExpr);
			}
			else if (mInfixFunctions.find(GetCurToken().type) != mInfixFunctions.end())
			{
				auto infixFn = mInfixFunctions[GetCurToken().type];
				leftExpr = (this->*infixFn)(leftExpr);
			}
			else
				break;
		}

		return leftExpr;
	}

	Expr *Parser::ParseIdentifierExpr()
	{
		auto identifierExpr = new IdentifierExpr();
		auto token = Consume(TOKEN_IDENTIFIER, L"Unexpect Identifier'" + GetCurToken().literal + L"'.");
		identifierExpr->line = token.line;
		identifierExpr->column = token.column;
		identifierExpr->literal = token.literal;
		return identifierExpr;
	}

	Expr *Parser::ParseNumExpr()
	{
		size_t line = GetCurToken().line;
		size_t column = GetCurToken().column;
		std::wstring numLiteral = Consume(TOKEN_NUMBER, L"Expexct a number literal.").literal;
		if (numLiteral.find('.') != std::wstring::npos)
		{
			auto realNumExpr = new RealNumExpr(std::stod(numLiteral));
			realNumExpr->line = line;
			realNumExpr->column = column;
			return realNumExpr;
		}
		else
		{
			auto intNumExpr = new IntNumExpr(std::stoll(numLiteral));
			intNumExpr->line = line;
			intNumExpr->column = column;
			return intNumExpr;
		}
	}

	Expr *Parser::ParseStrExpr()
	{
		auto token = Consume(TOKEN_STRING, L"Expect a string literal.");
		auto strExpr = new StrExpr();
		strExpr->column = token.column;
		strExpr->line = token.line;
		strExpr->value = token.literal;
		return strExpr;
	}

	Expr *Parser::ParseNullExpr()
	{
		auto token = Consume(TOKEN_NULL, L"Expect 'null' keyword");
		auto nullExpr = new NullExpr();
		nullExpr->line = token.line;
		nullExpr->column = token.column;
		return nullExpr;
	}
	Expr *Parser::ParseTrueExpr()
	{
		auto token = Consume(TOKEN_TRUE, L"Expect 'true' keyword");
		auto trueExpr = new BoolExpr(true);
		trueExpr->line = token.line;
		trueExpr->column = token.column;
		return trueExpr;
	}
	Expr *Parser::ParseFalseExpr()
	{
		auto token = Consume(TOKEN_FALSE, L"Expect 'false' keyword");
		auto falseExpr = new BoolExpr(false);
		falseExpr->line = token.line;
		falseExpr->column = token.column;
		return falseExpr;
	}

	Expr *Parser::ParseGroupExpr()
	{
		auto token = Consume(TOKEN_LPAREN, L"Expect '('.");
		auto groupExpr = new GroupExpr();
		groupExpr->column = token.column;
		groupExpr->line = token.line;
		groupExpr->expr = ParseExpr(Precedence::INFIX);
		Consume(TOKEN_RPAREN, L"Expect ')'.");
		return groupExpr;
	}

	Expr *Parser::ParseArrayExpr()
	{
		auto arrayExpr = new ArrayExpr();
		arrayExpr->column = GetCurToken().column;
		arrayExpr->line = GetCurToken().line;

		Consume(TOKEN_LBRACKET, L"Expect '['.");

		if (!IsMatchCurToken(TOKEN_RBRACKET))
		{
			// first element
			arrayExpr->elements.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
				arrayExpr->elements.emplace_back(ParseExpr());
		}

		Consume(TOKEN_RBRACKET, L"Expect ']'.");

		return arrayExpr;
	}

	Expr *Parser::ParseTableExpr()
	{
		auto tableExpr = new TableExpr();
		tableExpr->column = GetCurToken().column;
		tableExpr->line = GetCurToken().line;
		Consume(TOKEN_LBRACE, L"Expect '{'.");

		std::unordered_map<Expr *, Expr *> elements;

		if (!IsMatchCurToken(TOKEN_RBRACE))
		{
			Expr *key = ParseExpr();
			Consume(TOKEN_COLON, L"Expect ':' after table key.");
			Expr *value = ParseExpr();
			elements[key] = value;
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
			{
				Expr *key = ParseExpr();
				Consume(TOKEN_COLON, L"Expect ':' after table key.");
				Expr *value = ParseExpr();
				elements[key] = value;
			}
		}
		Consume(TOKEN_RBRACE, L"Expect '}' after table.");
		tableExpr->elements = elements;
		return tableExpr;
	}

	Expr *Parser::ParsePrefixExpr()
	{
		auto prefixExpr = new PrefixExpr();
		prefixExpr->column = GetCurToken().column;
		prefixExpr->line = GetCurToken().line;
		prefixExpr->op = GetCurTokenAndStepOnce().literal;
		prefixExpr->right = ParseExpr(Precedence::PREFIX);
		return prefixExpr;
	}

	Expr *Parser::ParseRefExpr()
	{
		auto token = Consume(TOKEN_AMPERSAND, L"Expect '&'.");
		auto refExpr = new RefExpr();
		refExpr->line = token.line;
		refExpr->column = token.column;
		refExpr->refExpr = ParseExpr(Precedence::PREFIX);
		return refExpr;
	}

	Expr *Parser::ParseInfixExpr(Expr *prefixExpr)
	{
		auto infixExpr = new InfixExpr();
		infixExpr->column = GetCurToken().column;
		infixExpr->line = GetCurToken().line;

		infixExpr->left = prefixExpr;

		Precedence opPrece = GetCurTokenPrecedence();

		infixExpr->op = GetCurTokenAndStepOnce().literal;
		infixExpr->right = ParseExpr(opPrece);
		return infixExpr;
	}

	Expr *Parser::ParsePostfixExpr(Expr *prefixExpr)
	{
		auto postfixExpr = new PostfixExpr();
		postfixExpr->column = GetCurToken().column;
		postfixExpr->line = GetCurToken().line;
		postfixExpr->op = GetCurTokenAndStepOnce().literal;
		postfixExpr->left = prefixExpr;
		return postfixExpr;
	}

	Expr *Parser::ParseConditionExpr(Expr *prefixExpr)
	{
		ConditionExpr *conditionExpr = new ConditionExpr();

		conditionExpr->column = GetCurToken().column;
		conditionExpr->line = GetCurToken().line;

		conditionExpr->condition = prefixExpr;

		Consume(TOKEN_QUESTION, L"Expect '?'.");

		conditionExpr->trueBranch = ParseExpr(Precedence::CONDITION);
		Consume(TOKEN_COLON, L"Expect ':' in condition expr");
		conditionExpr->falseBranch = ParseExpr(Precedence::CONDITION);

		return conditionExpr;
	}

	Expr *Parser::ParseIndexExpr(Expr *prefixExpr)
	{
		auto indexExpr = new IndexExpr();
		indexExpr->column = GetCurToken().column;
		indexExpr->line = GetCurToken().line;

		Consume(TOKEN_LBRACKET, L"Expect '['.");
		indexExpr->ds = prefixExpr;
		indexExpr->index = ParseExpr();
		Consume(TOKEN_RBRACKET, L"Expect ']'.");
		return indexExpr;
	}

	Expr *Parser::ParseFunctionCallExpr(Expr *prefixExpr)
	{
		auto funcCallExpr = new FunctionCallExpr();
		funcCallExpr->column = GetCurToken().column;
		funcCallExpr->line = GetCurToken().line;

		funcCallExpr->name = prefixExpr;
		Consume(TOKEN_LPAREN, L"Expect '('.");
		if (!IsMatchCurToken(TOKEN_RPAREN)) // has arguments
		{
			funcCallExpr->arguments.emplace_back(ParseExpr());
			while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
				funcCallExpr->arguments.emplace_back(ParseExpr());
		}
		Consume(TOKEN_RPAREN, L"Expect ')'.");

		return funcCallExpr;
	}

	Expr *Parser::ParseFieldCallExpr(Expr *prefixExpr)
	{
		auto fieldCallExpr = new FieldCallExpr();
		fieldCallExpr->column = GetCurToken().column;
		fieldCallExpr->line = GetCurToken().line;
		Consume(TOKEN_DOT, L"Expect '.'.");
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

	Token Parser::Consume(TokenType type, std::wstring_view errMsg)
	{
		if (IsMatchCurToken(type))
			return GetCurTokenAndStepOnce();
		Token token = GetCurToken();
		Assert(L"[line:" + std::to_wstring(token.line) + L",column:" + std::to_wstring(token.column) + L"]:" + std::wstring(errMsg));
		// avoid C++ compiler warning
		return Token(TOKEN_END, L"", -1, -1);
	}

	Token Parser::Consume(const std::vector<TokenType> &types, std::wstring_view errMsg)
	{
		for (const auto &type : types)
			if (IsMatchCurToken(type))
				return GetCurTokenAndStepOnce();
		Token token = GetCurToken();
		Assert(L"[" + std::to_wstring(token.line) + L"," + std::to_wstring(token.column) + L"]:" + std::wstring(errMsg));
		// avoid C++ compiler warning
		return Token(TOKEN_END, L"", -1, -1);
	}

	bool Parser::IsAtEnd()
	{
		return mCurPos >= (int32_t)mTokens.size();
	}
}