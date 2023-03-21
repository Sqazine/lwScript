#include "Parser.h"
#include "Library.h"
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
		{TOKEN_BANG, Precedence::POSTFIX}};

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
			{TOKEN_NEW, &Parser::ParseNewExpr},
			{TOKEN_THIS, &Parser::ParseThisExpr},
			{TOKEN_BASE, &Parser::ParseBaseExpr},
			{TOKEN_MATCH, &Parser::ParseMatchExpr},
			{TOKEN_LPAREN_LBRACE, &Parser::ParseBlockExpr},
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
			{TOKEN_LPAREN, &Parser::ParseCallExpr},
			{TOKEN_LBRACKET, &Parser::ParseIndexExpr},
			{TOKEN_DOT, &Parser::ParseDotExpr},
	};

	std::unordered_map<TokenType, PostfixFn> Parser::mPostfixFunctions =
		{
			{TOKEN_PLUS_PLUS, &Parser::ParsePostfixExpr},
			{TOKEN_MINUS_MINUS, &Parser::ParsePostfixExpr},
			{TOKEN_BANG, &Parser::ParsePostfixExpr},
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

		while (!IsMatchCurToken(TOKEN_EOF))
			mStmts->stmts.emplace_back(ParseDeclaration());
		return mStmts;
	}

	void Parser::ResetStatus()
	{
		mCurPos = 0;
		mLoopDepth = 0;
		mCurClassInfo = nullptr;

		if (mStmts != nullptr)
		{
			delete mStmts;
			mStmts = nullptr;
		}
		mStmts = new AstStmts();
	}

	Stmt *Parser::ParseDeclaration()
	{
		switch (GetCurToken().type)
		{
		case TOKEN_LET:
			return ParseLetDeclaration();
		case TOKEN_CONST:
			return ParseConstDeclaration();
		case TOKEN_FUNCTION:
			GetCurTokenAndStepOnce();
			return ParseFunctionDeclaration();
		case TOKEN_CLASS:
			return ParseClassDeclaration();
		case TOKEN_ENUM:
			return ParseEnumDeclaration();
		case TOKEN_MODULE:
			return ParseModuleDeclaration();
		default:
			return ParseStmt();
		}
	}
	Stmt *Parser::ParseLetDeclaration()
	{
		auto letStmt = new LetStmt();
		letStmt->line = GetCurToken().line;
		letStmt->column = GetCurToken().column;

		Consume(TOKEN_LET, L"Expect 'let' key word");

		std::unordered_map<IdentifierExpr *, VarDesc> variables;

		// variable name
		auto name = (IdentifierExpr *)ParseIdentifierExpr();

		// variable type
		std::wstring type = L"any";
		if (IsMatchCurToken(TOKEN_COLON))
			type = GetCurTokenAndStepOnce().literal;

		// variable value
		Expr *value = new NullExpr();
		if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
			value = ParseExpr();
		variables[name] = {.type = type, .value = value};

		while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
		{
			// variable name
			auto name = (IdentifierExpr *)ParseIdentifierExpr();

			// variable type
			type = L"any";
			if (IsMatchCurToken(TOKEN_COLON))
				type = GetCurTokenAndStepOnce().literal;

			Expr *value = new NullExpr();
			if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
				value = ParseExpr();
			variables[name] = {.type = type, .value = value};
		}

		Consume(TOKEN_SEMICOLON, L"Expect ';' after let declaration.");

		letStmt->variables = variables;

		return letStmt;
	}
	Stmt *Parser::ParseConstDeclaration()
	{
		auto constStmt = new ConstStmt();
		constStmt->line = GetCurToken().line;
		constStmt->column = GetCurToken().column;

		Consume(TOKEN_CONST, L"Expect 'const' key word");

		std::unordered_map<IdentifierExpr *, VarDesc> consts;

		auto name = (IdentifierExpr *)ParseIdentifierExpr();

		// variable type
		std::wstring type = L"any";
		if (IsMatchCurToken(TOKEN_COLON))
			type = GetCurTokenAndStepOnce().literal;

		Expr *value = new NullExpr();
		if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
			value = ParseExpr();
		consts[name] = {.type = type, .value = value};

		while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA))
		{
			auto name = (IdentifierExpr *)ParseIdentifierExpr();

			// variable type
			type = L"any";
			if (IsMatchCurToken(TOKEN_COLON))
				type = GetCurTokenAndStepOnce().literal;

			Expr *value = new NullExpr();
			if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
				value = ParseExpr();
			consts[name] = {.type = type, .value = value};
		}

		Consume(TOKEN_SEMICOLON, L"Expect ';' after const declaration.");

		constStmt->consts = consts;

		return constStmt;
	}
	Stmt *Parser::ParseFunctionDeclaration()
	{
		auto funcStmt = new FunctionStmt();
		funcStmt->line = GetCurToken().line;
		funcStmt->column = GetCurToken().column;

		funcStmt->name = (IdentifierExpr *)ParseIdentifierExpr();

		if (mCurClassInfo)
		{
			funcStmt->type = FunctionType::CLASS_CLOSURE;
			if (mCurClassInfo->name == funcStmt->name->literal)
				funcStmt->type = FunctionType::CLASS_CONSTRUCTOR;
		}

		Consume(TOKEN_LPAREN, L"Expect '(' after 'fn' keyword");

		if (!IsMatchCurToken(TOKEN_RPAREN)) // has parameter
		{
			do
			{
				IdentifierExpr *idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
				funcStmt->parameters.emplace_back(idenExpr);
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
		}
		Consume(TOKEN_RPAREN, L"Expect ')' after function stmt's '('");

		funcStmt->body = (ScopeStmt *)ParseScopeStmt();

		return funcStmt;
	}
	Stmt *Parser::ParseClassDeclaration()
	{
		auto classStmt = new ClassStmt();
		classStmt->line = GetCurToken().line;
		classStmt->column = GetCurToken().column;

		Consume(TOKEN_CLASS, L"Expect 'class' keyword");

		classStmt->name = ((IdentifierExpr *)ParseIdentifierExpr())->literal;

		ClassInfo classInfo;
		classInfo.hasSuperClass = false;
		classInfo.enclosing = mCurClassInfo;
		classInfo.name = classStmt->name;
		mCurClassInfo = &classInfo;

		if (IsMatchCurTokenAndStepOnce(TOKEN_COLON))
		{
			do
			{
				classStmt->parentClasses.emplace_back((IdentifierExpr *)ParseIdentifierExpr());
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));

			mCurClassInfo->hasSuperClass = true;
		}

		Consume(TOKEN_LBRACE, L"Expect '{' after class name or parent class name");

		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			if (IsMatchCurToken(TOKEN_LET))
				classStmt->letStmts.emplace_back((LetStmt *)ParseLetDeclaration());
			else if (IsMatchCurToken(TOKEN_CONST))
				classStmt->constStmts.emplace_back((ConstStmt *)ParseConstDeclaration());
			else if (IsMatchCurTokenAndStepOnce(TOKEN_FUNCTION))
			{
				auto fn = (FunctionStmt *)ParseFunctionDeclaration();
				if (fn->name->literal == classStmt->name)
					ASSERT(L"The class name conflicts with the class member function name");
				classStmt->fnStmts.emplace_back(fn);
			}
			else if (GetCurToken().literal == classStmt->name) // constructor
			{
				auto fn = (FunctionStmt *)ParseFunctionDeclaration();
				classStmt->constructors.emplace_back(fn);
			}
			else
				Consume({TOKEN_LET, TOKEN_FUNCTION, TOKEN_CONST}, L"UnExpect identifier '" + GetCurToken().literal + L"'.");
		}

		Consume(TOKEN_RBRACE, L"Expect '}' after class stmt's '{'");

		mCurClassInfo = mCurClassInfo->enclosing;

		return classStmt;
	}
	Stmt *Parser::ParseEnumDeclaration()
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

	Stmt *Parser::ParseModuleDeclaration()
	{
		Consume(TOKEN_MODULE, L"Expect 'module' keyword.");

		auto moduleDecl = new ModuleStmt();

		moduleDecl->modName = (IdentifierExpr *)ParseIdentifierExpr();

		Consume(TOKEN_LBRACE, L"Expect '{' after module name.");

		do
		{
			moduleDecl->modItems.emplace_back(ParseDeclaration());
		} while (!IsMatchCurToken(TOKEN_RBRACE));

		Consume(TOKEN_RBRACE, L"Expect '}'.");

		return moduleDecl;
	}

	Stmt *Parser::ParseStmt()
	{
		switch (GetCurToken().type)
		{
		case TOKEN_RETURN:
			return ParseReturnStmt();
		case TOKEN_IF:
			return ParseIfStmt();
		case TOKEN_LBRACE:
			return ParseScopeStmt();
		case TOKEN_WHILE:
			return ParseWhileStmt();
		case TOKEN_FOR:
			return ParseForStmt();
		case TOKEN_BREAK:
			return ParseBreakStmt();
		case TOKEN_CONTINUE:
			return ParseContinueStmt();
		case TOKEN_SWITCH:
			return ParseSwitchStmt();
		default:
			return ParseExprStmt();
		}
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
			scopeStmt->stmts.emplace_back(ParseDeclaration());
		Consume(TOKEN_RBRACE, L"Expect '}'.");
		return scopeStmt;
	}

	Stmt *Parser::ParseWhileStmt()
	{
		mLoopDepth++;

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

		mLoopDepth--;

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

		mLoopDepth++;

		auto scopeStmt = new ScopeStmt();
		scopeStmt->line = GetCurToken().line;
		scopeStmt->column = GetCurToken().column;

		Consume(TOKEN_FOR, L"Expect 'for' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'for'.");

		// initializer
		if (IsMatchCurToken(TOKEN_LET))
			scopeStmt->stmts.emplace_back(ParseLetDeclaration());
		else if (IsMatchCurToken(TOKEN_CONST))
			scopeStmt->stmts.emplace_back(ParseConstDeclaration());
		else
		{
			if (!IsMatchCurToken(TOKEN_SEMICOLON))
			{
				do
				{
					scopeStmt->stmts.emplace_back(new ExprStmt(ParseExpr()));
				} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
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
			do
			{
				increment.emplace_back(ParseExpr());
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
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
		mLoopDepth--;
		return scopeStmt;
	}

	Stmt *Parser::ParseBreakStmt()
	{
		if (mLoopDepth == 0)
			ASSERT(L"Cannot use 'break' stmt outside of 'for' or 'while' loop.")

		auto breakStmt = new BreakStmt();
		breakStmt->line = GetCurToken().line;
		breakStmt->column = GetCurToken().column;
		Consume(TOKEN_BREAK, L"Expect 'break' keyword.");
		Consume(TOKEN_SEMICOLON, L"Expect ';' after 'break' keyword.");
		return breakStmt;
	}

	Stmt *Parser::ParseContinueStmt()
	{
		if (mLoopDepth == 0)
			ASSERT(L"Cannot use 'break' stmt outside of 'for' or 'while' loop.")
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

		using Item = std::pair<std::vector<Expr *>, ScopeStmt *>;

		std::vector<Item> items;
		ScopeStmt *defaultScopeStmt = nullptr;
		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			if (IsMatchCurTokenAndStepOnce(TOKEN_DEFAULT))
			{
				Consume(TOKEN_COLON, L"Expect ':' after condition expr.");
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
				Item item;

				std::vector<Expr *> conditions;
				do
				{
					auto valueCompareExpr = ParseExpr();
					auto caseCondition = new InfixExpr(L"==", switchExpr, valueCompareExpr);
					caseCondition->line = GetCurToken().line;
					caseCondition->column = GetCurToken().column;
					conditions.emplace_back(caseCondition);
				} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
				Consume(TOKEN_COLON, L"Expect ':' after condition expr.");

				item.first = conditions;
				item.second = new ScopeStmt();
				item.second->line = GetCurToken().line;
				item.second->column = GetCurToken().column;

				if (IsMatchCurTokenAndStepOnce(TOKEN_LBRACE))
				{
					while (!IsMatchCurToken(TOKEN_RBRACE))
						item.second->stmts.emplace_back(ParseStmt());
					Consume(TOKEN_RBRACE, L"Expect '}' at the end of block while has multiple statements.");
				}
				else
					item.second->stmts.emplace_back(ParseStmt());

				items.emplace_back(item);
			}
		}

		Consume(TOKEN_RBRACE, L"Expect '}' after switch stmt");

		if (items.empty() && defaultScopeStmt != nullptr)
			return defaultScopeStmt;
		else
		{
			auto loopIfStmt = ifStmt;
			for (size_t i = 0; i < items.size(); ++i)
			{
				if (items[i].first.size() == 1)
					loopIfStmt->condition = items[i].first[0];
				else
				{
					Expr *condition = items[i].first[0];
					for (size_t j = 1; j < items[i].first.size(); ++j)
						condition = new InfixExpr(L"||", condition, items[i].first[j]);
					loopIfStmt->condition = condition;
				}
				loopIfStmt->thenBranch = items[i].second;
				if (i + 1 < items.size())
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

	Expr *Parser::ParseLambdaExpr()
	{
		auto lambdaExpr = new LambdaExpr();
		lambdaExpr->line = GetCurToken().line;
		lambdaExpr->column = GetCurToken().column;

		Consume(TOKEN_FUNCTION, L"Expect 'fn' keyword");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'fn' keyword");

		if (!IsMatchCurToken(TOKEN_RPAREN)) // has parameter
		{
			do
			{
				IdentifierExpr *idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
				lambdaExpr->parameters.emplace_back(idenExpr);
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
		}
		Consume(TOKEN_RPAREN, L"Expect ')' after lambda expr's '('");

		lambdaExpr->body = (ScopeStmt *)ParseScopeStmt();

		return lambdaExpr;
	}

	Expr *Parser::ParseNewExpr()
	{
		auto newExpr = new NewExpr();
		newExpr->line = GetCurToken().line;
		newExpr->column = GetCurToken().column;

		Consume(TOKEN_NEW, L"Expect 'new' keyword");

		auto callee = ParseExpr();

		if (callee->type != AST_CALL)
			ASSERT(L"Not a valid new expr,call expr is is necessary followed 'new' keyword.");

		newExpr->callee = (CallExpr *)callee;
		return newExpr;
	}

	Expr *Parser::ParseThisExpr()
	{
		Consume(TOKEN_THIS, L"Expect 'this' keyword");
		if (!mCurClassInfo)
			ASSERT(L"Invalid 'this' keyword:Cannot use 'this' outside class.")
		return new ThisExpr();
	}

	Expr *Parser::ParseBaseExpr()
	{
		Consume(TOKEN_BASE, L"Expect 'base' keyword");
		if (!mCurClassInfo)
			ASSERT(L"Invalid 'base' keyword:Cannot use 'base' outside class.")

		Consume(TOKEN_DOT, L"Expect '.' after 'base' keyword");

		return new BaseExpr((IdentifierExpr *)ParseIdentifierExpr());
	}

	Expr *Parser::ParseMatchExpr()
	{
		Expr *defaultBranch = nullptr;

		Consume(TOKEN_MATCH, L"Expect 'match' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'match' keyword.");
		auto judgeExpr = ParseExpr();
		judgeExpr->column = GetCurToken().column;
		judgeExpr->line = GetCurToken().line;
		Consume(TOKEN_RPAREN, L"Expect ')' after match's expression.");
		Consume(TOKEN_LBRACE, L"Expect '{' after 'match' keyword.");

		using Item = std::pair<std::vector<Expr *>, Expr *>;

		std::vector<Item> items;
		bool hasDefaultBranch = false;
		if (!IsMatchCurToken(TOKEN_RBRACE))
		{
			do
			{
				if (IsMatchCurTokenAndStepOnce(TOKEN_DEFAULT))
				{
					if (hasDefaultBranch)
						ASSERT(L"Already exists a default branch.only a default branch is available in a match expr.");
					Consume(TOKEN_COLON, L"Expect ':' after default's condition expr.");
					defaultBranch = ParseExpr();
					defaultBranch->line = GetCurToken().line;
					defaultBranch->column = GetCurToken().column;
					hasDefaultBranch = true;
				}
				else
				{
					Item item;

					std::vector<Expr *> conditions;

					do
					{
						auto valueCompareExpr = ParseExpr();
						auto caseCondition = new InfixExpr(L"==", judgeExpr, valueCompareExpr);
						caseCondition->line = GetCurToken().line;
						caseCondition->column = GetCurToken().column;
						conditions.emplace_back(caseCondition);
					} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
					Consume(TOKEN_COLON, L"Expect ':' after match item's condition expr.");

					item.first = conditions;
					item.second = ParseExpr();
					item.second->line = GetCurToken().line;
					item.second->column = GetCurToken().column;

					items.emplace_back(item);
				}
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
		}

		Consume(TOKEN_RBRACE, L"Expect '}' after match expr");

		auto conditionExpr = new ConditionExpr();
		auto loopConditionExpr = conditionExpr;
		for (int32_t i = 0; i < items.size(); ++i)
		{
			if (items[i].first.size() == 1)
				loopConditionExpr->condition = items[i].first[0];
			else
			{
				Expr *fullCondition = items[i].first[0];
				for (size_t j = 1; j < items[i].first.size(); ++j)
					fullCondition = new InfixExpr(L"||", fullCondition, items[i].first[j]);
				loopConditionExpr->condition = fullCondition;
			}

			loopConditionExpr->trueBranch = items[i].second;
			if (i + 1 < items.size())
			{
				loopConditionExpr->falseBranch = new ConditionExpr();
				loopConditionExpr = (ConditionExpr *)loopConditionExpr->falseBranch;
			}
		}

		if (defaultBranch)
			loopConditionExpr->falseBranch = defaultBranch;

		return conditionExpr;
	}

	Expr *Parser::ParseBlockExpr()
	{
		Consume(TOKEN_LPAREN_LBRACE, L"Expect '({'.");

		mSkippingConsumeTokenTypeStack.emplace_back(TOKEN_SEMICOLON);

		auto blockExpr = new BlockExpr();

		std::vector<struct Stmt *> stmts;
		do
		{
			if(IsMatchCurToken(TOKEN_RBRACE_RPAREN))
				ASSERT("Expr required at the end of block expression.");
			stmts.emplace_back(ParseDeclaration());
		} while (IsMatchCurTokenAndStepOnce(TOKEN_SEMICOLON));

		mSkippingConsumeTokenTypeStack.pop_back();

		if(stmts.back()->type!=AST_EXPR)
			ASSERT("Expr required at the end of block expression.");

		auto expr=((ExprStmt*)stmts.back())->expr;
		stmts.pop_back();

		blockExpr->stmts=stmts;
		blockExpr->endExpr=expr;

		Consume(TOKEN_RBRACE_RPAREN, L"Expect '})'.");

		return blockExpr;
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
		groupExpr->expr = ParseExpr();
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
			do
			{
				arrayExpr->elements.emplace_back(ParseExpr());
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
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

		std::vector<std::pair<Expr *, Expr *>> elements;

		if (!IsMatchCurToken(TOKEN_RBRACE))
		{
			do
			{
				Expr *key = ParseExpr();
				Consume(TOKEN_COLON, L"Expect ':' after table key.");
				Expr *value = ParseExpr();
				elements.emplace_back(std::make_pair(key, value));
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
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

	Expr *Parser::ParseCallExpr(Expr *prefixExpr)
	{
		auto callExpr = new CallExpr();
		callExpr->column = GetCurToken().column;
		callExpr->line = GetCurToken().line;

		callExpr->callee = (IdentifierExpr *)prefixExpr;
		Consume(TOKEN_LPAREN, L"Expect '('.");
		if (!IsMatchCurToken(TOKEN_RPAREN)) // has arguments
		{
			do
			{
				callExpr->arguments.emplace_back(ParseExpr());
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
		}
		Consume(TOKEN_RPAREN, L"Expect ')'.");

		return callExpr;
	}

	Expr *Parser::ParseDotExpr(Expr *prefixExpr)
	{
		auto dotExpr = new DotExpr();
		dotExpr->column = GetCurToken().column;
		dotExpr->line = GetCurToken().line;
		Consume(TOKEN_DOT, L"Expect '.'.");
		dotExpr->callee = prefixExpr;
		dotExpr->callMember = (IdentifierExpr *)ParseIdentifierExpr();
		return dotExpr;
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
		if (mSkippingConsumeTokenTypeStack.empty()||type != mSkippingConsumeTokenTypeStack.back())
		{
			if (IsMatchCurToken(type))
				return GetCurTokenAndStepOnce();
			Token token = GetCurToken();
			ASSERT(L"[line:" + std::to_wstring(token.line) + L",column:" + std::to_wstring(token.column) + L"]:" + std::wstring(errMsg))
		}
		// avoid C++ compiler warning
		return Token(TOKEN_EOF, L"", -1, -1);
	}

	Token Parser::Consume(const std::vector<TokenType> &types, std::wstring_view errMsg)
	{
		if(!mSkippingConsumeTokenTypeStack.empty())
			for (const auto &type : types)
				if (type == mSkippingConsumeTokenTypeStack.back())
					return Token(TOKEN_EOF, L"", -1, -1);

		for (const auto &type : types)
			if (IsMatchCurToken(type))
				return GetCurTokenAndStepOnce();
		Token token = GetCurToken();
		ASSERT(L"[" + std::to_wstring(token.line) + L"," + std::to_wstring(token.column) + L"]:" + std::wstring(errMsg))

		// avoid C++ compiler warning
		return Token(TOKEN_EOF, L"", -1, -1);
	}

	bool Parser::IsAtEnd()
	{
		return mCurPos >= (int32_t)mTokens.size();
	}
}