#include "Parser.h"
#include "Library.h"
namespace lws
{
	struct PrecedenceBinding
	{
		TokenType type;
		Precedence precedence;
	};

	constexpr PrecedenceBinding precedenceDict[] = {
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
		{TOKEN_BANG, Precedence::INFIX},
		{TOKEN_PLUS_PLUS, Precedence::POSTFIX},
		{TOKEN_MINUS_MINUS, Precedence::POSTFIX},
	};

	struct AssociativityBinding
	{
		Precedence precedence;
		Associativity associativity;
	};

	constexpr AssociativityBinding associativityDict[] =
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
			{TOKEN_LBRACE, &Parser::ParseDictExpr},
			{TOKEN_AMPERSAND, &Parser::ParseRefExpr},
			{TOKEN_FUNCTION, &Parser::ParseLambdaExpr},
			{TOKEN_PLUS_PLUS, &Parser::ParsePrefixExpr},
			{TOKEN_MINUS_MINUS, &Parser::ParsePrefixExpr},
			{TOKEN_NEW, &Parser::ParseNewExpr},
			{TOKEN_THIS, &Parser::ParseThisExpr},
			{TOKEN_BASE, &Parser::ParseBaseExpr},
			{TOKEN_MATCH, &Parser::ParseMatchExpr},
			{TOKEN_LPAREN_LBRACE, &Parser::ParseBlockExpr},
			{TOKEN_ELLIPSIS, &Parser::ParseVarArgExpr},
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
			{TOKEN_BANG, &Parser::ParseFactorialExpr},
	};

	std::unordered_map<TokenType, PostfixFn> Parser::mPostfixFunctions =
		{
			{TOKEN_PLUS_PLUS, &Parser::ParsePostfixExpr},
			{TOKEN_MINUS_MINUS, &Parser::ParsePostfixExpr},
	};

	Parser::Parser()
		: mCurClassInfo(nullptr), mCurPos(0)
	{
	}

	Parser::~Parser()
	{
		std::unordered_map<TokenType, PrefixFn>().swap(mPrefixFunctions);
		std::unordered_map<TokenType, InfixFn>().swap(mInfixFunctions);
	}

	Stmt *Parser::Parse(const std::vector<Token> &tokens)
	{
		ResetStatus();
		mTokens = tokens;

		AstStmts *astStmts = new AstStmts(GetCurToken());

		while (!IsMatchCurToken(TOKEN_EOF))
			astStmts->stmts.emplace_back(ParseDecl());

		auto result = mSyntaxChecker.Check(astStmts);
		result = mOptimizer.Opt(result);

		return result;
	}

	void Parser::ResetStatus()
	{
		mCurPos = 0;

		mCurClassInfo = nullptr;
	}

	Stmt *Parser::ParseDecl()
	{
		switch (GetCurToken().type)
		{
		case TOKEN_LET:
		case TOKEN_CONST:
			return ParseVarDecl(GetCurToken().type);
		case TOKEN_FUNCTION:
			GetCurTokenAndStepOnce();
			return ParseFunctionDecl();
		case TOKEN_CLASS:
			return ParseClassDecl();
		case TOKEN_ENUM:
			return ParseEnumDecl();
		case TOKEN_MODULE:
			return ParseModuleDecl();
		default:
			return ParseStmt();
		}
	}

	Stmt *Parser::ParseVarDecl(TokenType tType)
	{
		auto varStmt = new VarStmt(GetCurToken());

		if (tType == TOKEN_LET)
			varStmt->privilege = VarStmt::Privilege::MUTABLE;
		else if (tType == TOKEN_CONST)
			varStmt->privilege = VarStmt::Privilege::IMMUTABLE;

		Consume(tType, L"Expect 'let' or 'const' key word");

		do
		{
			if (IsMatchCurToken(TOKEN_LBRACKET))
				varStmt->variables.emplace_back(ParseDestructuringAssignmentExpr());
			else
			{
				auto varDescExpr = ParseVarDescExpr();

				Expr *value = new NullExpr(varDescExpr->tagToken);
				if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
					value = ParseExpr();
				varStmt->variables.emplace_back(varDescExpr, value);
			}

		} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));

		Consume(TOKEN_SEMICOLON, L"Expect ';' after let or const declaration.");

		return varStmt;
	}

	Stmt *Parser::ParseFunctionDecl()
	{
		auto funcStmt = new FunctionStmt(GetCurToken());

		{
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
					funcStmt->parameters.emplace_back((VarDescExpr *)ParseVarDescExpr());
				} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
			}

			Consume(TOKEN_RPAREN, L"Expect ')' after function stmt's '('");

			funcStmt->body = (ScopeStmt *)ParseScopeStmt();

			if (funcStmt->body->stmts.back()->type != AST_RETURN && funcStmt->type != FunctionType::CLASS_CONSTRUCTOR)
			{
				auto tmpReturn = new ReturnStmt(GetCurToken());
				funcStmt->body->stmts.emplace_back(tmpReturn);
			}
		}

		return funcStmt;
	}

	Stmt *Parser::ParseClassDecl()
	{
		auto classStmt = new ClassStmt(GetCurToken());

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
				classStmt->varStmts.emplace_back((VarStmt *)ParseVarDecl(TOKEN_LET));
			else if (IsMatchCurToken(TOKEN_CONST))
				classStmt->varStmts.emplace_back((VarStmt *)ParseVarDecl(TOKEN_CONST));
			else if (IsMatchCurTokenAndStepOnce(TOKEN_FUNCTION))
			{
				auto fn = (FunctionStmt *)ParseFunctionDecl();
				if (fn->name->literal == classStmt->name)
					Hint::Error(fn->name->tagToken, L"The class member function name :{} conflicts with its class:{}, only constructor function name is allowed to same with its class's name", fn->name->literal);
				classStmt->fnStmts.emplace_back(fn);
			}
			else if(IsMatchCurToken(TOKEN_ENUM))
				classStmt->enumStmts.emplace_back((EnumStmt*)ParseEnumDecl());
			else if (GetCurToken().literal == classStmt->name) // constructor
			{
				auto fn = (FunctionStmt *)ParseFunctionDecl();
				classStmt->constructors.emplace_back(fn);
			}
			else
				Consume({TOKEN_LET, TOKEN_FUNCTION, TOKEN_CONST}, L"UnExpect identifier '" + GetCurToken().literal + L"'.");
		}

		Consume(TOKEN_RBRACE, L"Expect '}' after class stmt's '{'");

		mCurClassInfo = mCurClassInfo->enclosing;

		return classStmt;
	}
	Stmt *Parser::ParseEnumDecl()
	{
		auto enumStmt = new EnumStmt(GetCurToken());

		Consume(TOKEN_ENUM, L"Expect 'enum' keyword.");
		enumStmt->name = (IdentifierExpr *)ParseIdentifierExpr();
		Consume(TOKEN_LBRACE, L"Expect '{' after 'enum' keyword.");

		std::unordered_map<IdentifierExpr *, Expr *> items;

		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			auto name = (IdentifierExpr *)ParseIdentifierExpr();
			Expr *value = new StrExpr(name->tagToken, name->literal);
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

	Stmt *Parser::ParseModuleDecl()
	{
		auto token = Consume(TOKEN_MODULE, L"Expect 'module' keyword.");

		auto moduleDecl = new ModuleStmt(token);

		moduleDecl->name = (IdentifierExpr *)ParseIdentifierExpr();

		Consume(TOKEN_LBRACE, L"Expect '{' after module name.");

		while (!IsMatchCurToken(TOKEN_RBRACE))
		{
			switch (GetCurToken().type)
			{
			case TOKEN_LET:
			case TOKEN_CONST:
				moduleDecl->varItems.emplace_back((VarStmt *)ParseVarDecl(GetCurToken().type));
				break;
			case TOKEN_FUNCTION:
				GetCurTokenAndStepOnce();
				moduleDecl->functionItems.emplace_back((FunctionStmt *)ParseFunctionDecl());
				break;
			case TOKEN_CLASS:
				moduleDecl->classItems.emplace_back((ClassStmt *)ParseClassDecl());
				break;
			case TOKEN_ENUM:
				moduleDecl->enumItems.emplace_back((EnumStmt *)ParseEnumDecl());
				break;
			case TOKEN_MODULE:
				moduleDecl->moduleItems.emplace_back((ModuleStmt *)ParseModuleDecl());
				break;
			default:
				Hint::Error(GetCurToken(), L"Only let,const,function,class,enum and module is available in module scope");
			}
		}

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
		auto exprStmt = new ExprStmt(GetCurToken());
		exprStmt->expr = ParseExpr();

		Consume(TOKEN_SEMICOLON, L"Expect ';' after expr stmt.");
		return exprStmt;
	}

	Stmt *Parser::ParseReturnStmt()
	{
		auto returnStmt = new ReturnStmt(GetCurToken());

		Consume(TOKEN_RETURN, L"Expect 'return' key word.");

		if (!IsMatchCurToken(TOKEN_SEMICOLON))
		{
			std::vector<Expr *> returnExprs;
			do
			{
				if (IsMatchCurToken(TOKEN_SEMICOLON))
					break;

				returnExprs.emplace_back(ParseExpr());

			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));

			if (returnExprs.size() > 1)
				returnStmt->expr = new AppregateExpr(returnExprs[0]->tagToken, returnExprs);
			else if (returnExprs.size() == 1)
				returnStmt->expr = returnExprs[0];
		}

		Consume(TOKEN_SEMICOLON, L"Expect ';' after return stmt");
		return returnStmt;
	}

	Stmt *Parser::ParseIfStmt()
	{
		auto ifStmt = new IfStmt(GetCurToken());

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
		auto scopeStmt = new ScopeStmt(GetCurToken());

		Consume(TOKEN_LBRACE, L"Expect '{'.");
		while (!IsMatchCurToken(TOKEN_RBRACE))
			scopeStmt->stmts.emplace_back(ParseDecl());
		Consume(TOKEN_RBRACE, L"Expect '}'.");
		return scopeStmt;
	}

	Stmt *Parser::ParseWhileStmt()
	{
		auto whileStmt = new WhileStmt(GetCurToken());

		Consume(TOKEN_WHILE, L"Expect 'while' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'while'.");

		whileStmt->condition = ParseExpr();

		Consume(TOKEN_RPAREN, L"Expect ')' after while stmt's condition.");

		if (IsMatchCurToken(TOKEN_LBRACE)) // scope stmt:while(a<10){a=a+1;}
			whileStmt->body = (ScopeStmt *)ParseScopeStmt();
		else // single stmt:while(a<10) a=a+1;
		{
			auto scopeStmt = new ScopeStmt(GetCurToken());
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

		auto token = GetCurToken();

		auto scopeStmt = new ScopeStmt(token);

		Consume(TOKEN_FOR, L"Expect 'for' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'for'.");

		// initializer
		if (IsMatchCurToken(TOKEN_LET))
			scopeStmt->stmts.emplace_back(ParseVarDecl(TOKEN_LET));
		else if (IsMatchCurToken(TOKEN_CONST))
			scopeStmt->stmts.emplace_back(ParseVarDecl(TOKEN_CONST));
		else
		{
			if (!IsMatchCurToken(TOKEN_SEMICOLON))
			{
				do
				{
					scopeStmt->stmts.emplace_back(new ExprStmt(GetCurToken(), ParseExpr()));
				} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
			}
			Consume(TOKEN_SEMICOLON, L"Expect ';' after for stmt's initializer stmt");
		}

		Expr *condition = nullptr;
		if (!IsMatchCurToken(TOKEN_SEMICOLON))
			condition = ParseExpr();
		Consume(TOKEN_SEMICOLON, L"Expect ';' after for stmt's condition expr.");

		std::vector<Expr *> increment;
		auto incrementToken = GetCurToken();
		if (!IsMatchCurToken(TOKEN_RPAREN))
		{
			do
			{
				increment.emplace_back(ParseExpr());
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
		}
		Consume(TOKEN_RPAREN, L"Expect ')' after for stmt's increment expr(s)");
		auto scopeToken = GetCurToken();
		std::vector<Stmt *> whileBodyStmts;
		if (IsMatchCurToken(TOKEN_LBRACE)) // scope stmt for 'for' stmt:like for(a=0;a<10;a=a+1){println("{}",a);}
			whileBodyStmts.emplace_back(ParseScopeStmt());
		else // single stmt for 'for' stmt:like for(a=0;a<10;a=a+1) println("{}",a);
		{
			auto scopeStmt = new ScopeStmt(scopeToken);
			scopeStmt->stmts.emplace_back(ParseStmt());
			whileBodyStmts.emplace_back(scopeStmt);
		}

		auto whileStmt = new WhileStmt(token);
		whileStmt->condition = condition;
		whileStmt->body = new ScopeStmt(scopeToken, whileBodyStmts);
		std::vector<Stmt *> incrementStmts;
		for (const auto expr : increment)
			incrementStmts.emplace_back(new ExprStmt(expr->tagToken, expr));
		whileStmt->increment = new ScopeStmt(incrementToken, incrementStmts);
		scopeStmt->stmts.emplace_back(whileStmt);

		return scopeStmt;
	}

	Stmt *Parser::ParseBreakStmt()
	{
		auto breakStmt = new BreakStmt(GetCurToken());

		Consume(TOKEN_BREAK, L"Expect 'break' keyword.");
		Consume(TOKEN_SEMICOLON, L"Expect ';' after 'break' keyword.");
		return breakStmt;
	}

	Stmt *Parser::ParseContinueStmt()
	{
		auto continueStmt = new ContinueStmt(GetCurToken());

		Consume(TOKEN_CONTINUE, L"Expect 'continue' keyword");
		Consume(TOKEN_SEMICOLON, L"Expect ';' after 'continue' keyword.");
		return continueStmt;
	}

	Stmt *Parser::ParseSwitchStmt()
	{
		auto ifStmt = new IfStmt(GetCurToken());

		Consume(TOKEN_SWITCH, L"Expect 'switch' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'switch' keyword.");

		auto switchExpr = ParseIdentifierExpr();

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
				defaultScopeStmt = new ScopeStmt(GetCurToken());

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
					auto caseCondition = new InfixExpr(GetCurToken(), L"==", switchExpr, valueCompareExpr);
					conditions.emplace_back(caseCondition);
				} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
				Consume(TOKEN_COLON, L"Expect ':' after condition expr.");

				item.first = conditions;
				item.second = new ScopeStmt(GetCurToken());

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
						condition = new InfixExpr(condition->tagToken, L"||", condition, items[i].first[j]);
					loopIfStmt->condition = condition;
				}
				loopIfStmt->thenBranch = items[i].second;
				if (i + 1 < items.size())
				{
					loopIfStmt->elseBranch = new IfStmt(ifStmt->tagToken);
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
		auto lambdaExpr = new LambdaExpr(GetCurToken());

		Consume(TOKEN_FUNCTION, L"Expect 'fn' keyword");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'fn' keyword");

		if (!IsMatchCurToken(TOKEN_RPAREN)) // has parameter
		{
			do
			{
				lambdaExpr->parameters.emplace_back((VarDescExpr *)ParseVarDescExpr());
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
		}
		Consume(TOKEN_RPAREN, L"Expect ')' after lambda expr's '('");

		lambdaExpr->body = (ScopeStmt *)ParseScopeStmt();

		return lambdaExpr;
	}

	Expr *Parser::ParseNewExpr()
	{
		auto newExpr = new NewExpr(GetCurToken());

		Consume(TOKEN_NEW, L"Expect 'new' keyword");

		Expr *callee;
		if (IsMatchCurToken(TOKEN_LBRACE))
			callee = ParseAnonyObjExpr();
		else
			callee = ParseExpr();

		newExpr->callee = callee;
		return newExpr;
	}

	Expr *Parser::ParseThisExpr()
	{
		auto token = Consume(TOKEN_THIS, L"Expect 'this' keyword");

		auto thisExpr = new ThisExpr(token);

		if (!mCurClassInfo)
			Hint::Error(thisExpr->tagToken, L"Invalid 'this' keyword:Cannot use 'this' outside class.");

		return thisExpr;
	}

	Expr *Parser::ParseBaseExpr()
	{
		auto token = Consume(TOKEN_BASE, L"Expect 'base' keyword");

		Consume(TOKEN_DOT, L"Expect '.' after base keyword");

		auto baseExpr = new BaseExpr(token, (IdentifierExpr *)ParseIdentifierExpr());

		if (!mCurClassInfo)
			Hint::Error(baseExpr->tagToken, L"Invalid 'base' keyword:Cannot use 'base' outside class.");

		return baseExpr;
	}

	Expr *Parser::ParseMatchExpr()
	{
		Expr *defaultBranch = nullptr;

		auto matchToken = Consume(TOKEN_MATCH, L"Expect 'match' keyword.");
		Consume(TOKEN_LPAREN, L"Expect '(' after 'match' keyword.");

		auto judgeExpr = ParseExpr();

		Consume(TOKEN_RPAREN, L"Expect ')' after match's expression.");
		Consume(TOKEN_LBRACE, L"Expect '{' after 'match' keyword.");

		using Item = std::pair<std::vector<Expr *>, Expr *>;

		std::vector<Item> items;
		bool hasDefaultBranch = false;
		if (!IsMatchCurToken(TOKEN_RBRACE))
		{
			do
			{
				if (IsMatchCurToken(TOKEN_DEFAULT))
				{
					if (hasDefaultBranch)
						Hint::Error(GetCurToken(), L"Already exists a default branch.only a default branch is available in a match expr.");

					GetCurTokenAndStepOnce();

					Consume(TOKEN_COLON, L"Expect ':' after default's condition expr.");
					defaultBranch = ParseExpr();
					hasDefaultBranch = true;
				}
				else
				{
					Item item;

					std::vector<Expr *> conditions;

					do
					{
						auto valueCompareExpr = ParseExpr();
						auto caseCondition = new InfixExpr(GetCurToken(), L"==", judgeExpr, valueCompareExpr);
						conditions.emplace_back(caseCondition);
					} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
					Consume(TOKEN_COLON, L"Expect ':' after match item's condition expr.");

					item.first = conditions;
					item.second = ParseExpr();

					items.emplace_back(item);
				}
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
		}

		Consume(TOKEN_RBRACE, L"Expect '}' after match expr");

		auto conditionExpr = new ConditionExpr(matchToken);
		auto loopConditionExpr = conditionExpr;
		for (int32_t i = 0; i < items.size(); ++i)
		{
			if (items[i].first.size() == 1)
				loopConditionExpr->condition = items[i].first[0];
			else
			{
				Expr *fullCondition = items[i].first[0];
				for (size_t j = 1; j < items[i].first.size(); ++j)
					fullCondition = new InfixExpr(fullCondition->tagToken, L"||", fullCondition, items[i].first[j]);
				loopConditionExpr->condition = fullCondition;
			}

			loopConditionExpr->trueBranch = items[i].second;
			if (i + 1 < items.size())
			{
				loopConditionExpr->falseBranch = new ConditionExpr(items[i].second->tagToken);
				loopConditionExpr = (ConditionExpr *)loopConditionExpr->falseBranch;
			}
		}

		if (defaultBranch)
			loopConditionExpr->falseBranch = defaultBranch;

		return conditionExpr;
	}

	Expr *Parser::ParseBlockExpr()
	{
		auto token = Consume(TOKEN_LPAREN_LBRACE, L"Expect '({'.");

		mSkippingConsumeTokenTypeStack.emplace_back(TOKEN_SEMICOLON);

		auto blockExpr = new BlockExpr(token);

		std::vector<struct Stmt *> stmts;
		do
		{
			if (IsMatchCurToken(TOKEN_RBRACE_RPAREN))
				Hint::Error(GetCurToken(), L"Expr required at the end of block expression.");
			stmts.emplace_back(ParseDecl());
		} while (IsMatchCurTokenAndStepOnce(TOKEN_SEMICOLON));

		mSkippingConsumeTokenTypeStack.pop_back();

		if (stmts.back()->type != AST_EXPR)
			Hint::Error(stmts.back()->tagToken, L"Expr required at the end of block expression.");

		auto expr = ((ExprStmt *)stmts.back())->expr;
		stmts.pop_back();

		blockExpr->stmts = stmts;
		blockExpr->endExpr = expr;

		Consume(TOKEN_RBRACE_RPAREN, L"Expect '})'.");

		return blockExpr;
	}

	Expr *Parser::ParseExpr(Precedence precedence)
	{
		if (mPrefixFunctions.find(GetCurToken().type) == mPrefixFunctions.end())
		{
			auto token = GetCurTokenAndStepOnce();
			Hint::Error(token, L"no prefix definition for:{}", token.literal);

			auto nullExpr = new NullExpr(token);

			return nullExpr;
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
		auto token = Consume(TOKEN_IDENTIFIER, L"Unexpect Identifier'" + GetCurToken().literal + L"'.");
		auto identifierExpr = new IdentifierExpr(token);
		identifierExpr->literal = token.literal;
		return identifierExpr;
	}

	Expr *Parser::ParseNumExpr()
	{
		auto token = GetCurToken();
		std::wstring numLiteral = Consume(TOKEN_NUMBER, L"Expexct a number literal.").literal;
		if (numLiteral.find('.') != std::wstring::npos)
		{
			auto realNumExpr = new RealNumExpr(token, std::stod(numLiteral));
			return realNumExpr;
		}
		else
		{
			auto intNumExpr = new IntNumExpr(token, std::stoll(numLiteral));
			return intNumExpr;
		}
	}

	Expr *Parser::ParseStrExpr()
	{
		auto token = Consume(TOKEN_STRING, L"Expect a string literal.");
		auto strExpr = new StrExpr(token);

		strExpr->value = token.literal;
		return strExpr;
	}

	Expr *Parser::ParseNullExpr()
	{
		auto token = Consume(TOKEN_NULL, L"Expect 'null' keyword");
		auto nullExpr = new NullExpr(token);
		return nullExpr;
	}
	Expr *Parser::ParseTrueExpr()
	{
		auto token = Consume(TOKEN_TRUE, L"Expect 'true' keyword");
		auto trueExpr = new BoolExpr(token, true);
		return trueExpr;
	}
	Expr *Parser::ParseFalseExpr()
	{
		auto token = Consume(TOKEN_FALSE, L"Expect 'false' keyword");
		auto falseExpr = new BoolExpr(token, false);
		return falseExpr;
	}

	Expr *Parser::ParseGroupExpr()
	{
		auto token = Consume(TOKEN_LPAREN, L"Expect '('.");
		auto groupExpr = new GroupExpr(token);
		groupExpr->expr = ParseExpr();
		Consume(TOKEN_RPAREN, L"Expect ')'.");
		return groupExpr;
	}

	Expr *Parser::ParseArrayExpr()
	{
		auto arrayExpr = new ArrayExpr(GetCurToken());

		Consume(TOKEN_LBRACKET, L"Expect '['.");

		if (!IsMatchCurToken(TOKEN_RBRACKET))
		{
			do
			{
				if (IsMatchCurToken(TOKEN_RBRACKET))
					break;

				arrayExpr->elements.emplace_back(ParseExpr());
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));
		}

		Consume(TOKEN_RBRACKET, L"Expect ']'.");

		return arrayExpr;
	}

	Expr *Parser::ParseDictExpr()
	{
		auto dictExpr = new DictExpr(GetCurToken());

		Consume(TOKEN_LBRACE, L"Expect '{'.");

		if (!IsMatchCurToken(TOKEN_RBRACE))
		{
			std::vector<std::pair<Expr *, Expr *>> elements;
			do
			{
				if (IsMatchCurToken(TOKEN_RBRACE))
					break;

				Expr *key = ParseExpr();
				Consume(TOKEN_COLON, L"Expect ':' after dict key.");
				Expr *value = ParseExpr();
				elements.emplace_back(std::make_pair(key, value));
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));

			dictExpr->elements = elements;
		}
		Consume(TOKEN_RBRACE, L"Expect '}' after dict.");
		return dictExpr;
	}

	Expr *Parser::ParseAnonyObjExpr()
	{
		auto anonyObjExpr = new AnonyObjExpr(GetCurToken());

		Consume(TOKEN_LBRACE, L"Expect '{'.");

		if (!IsMatchCurToken(TOKEN_RBRACE))
		{
			std::vector<std::pair<std::wstring, Expr *>> elements;
			do
			{
				if (IsMatchCurToken(TOKEN_RBRACE))
					break;

				Expr *key = ParseExpr();

				if (key->type != AST_IDENTIFIER)
					Hint::Error(key->tagToken, L"Anonymous object require key must be a valid identifier.");

				Consume(TOKEN_COLON, L"Expect ':' after anony object key.");
				Expr *value = ParseExpr();
				elements.emplace_back(((IdentifierExpr *)key)->literal, value);
			} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));

			anonyObjExpr->elements = elements;
		}
		Consume(TOKEN_RBRACE, L"Expect '}' after anony object.");
		return anonyObjExpr;
	}

	Expr *Parser::ParsePrefixExpr()
	{
		auto prefixExpr = new PrefixExpr(GetCurToken());

		prefixExpr->op = GetCurTokenAndStepOnce().literal;
		prefixExpr->right = ParseExpr(Precedence::PREFIX);
		return prefixExpr;
	}

	Expr *Parser::ParseRefExpr()
	{
		auto token = Consume(TOKEN_AMPERSAND, L"Expect '&'.");
		auto refExpr = new RefExpr(token);

		refExpr->refExpr = ParseExpr(Precedence::PREFIX);
		return refExpr;
	}

	Expr *Parser::ParseInfixExpr(Expr *prefixExpr)
	{
		auto infixExpr = new InfixExpr(GetCurToken());
		infixExpr->left = prefixExpr;
		Precedence opPrece = GetCurTokenPrecedence();
		infixExpr->op = GetCurTokenAndStepOnce().literal;
		infixExpr->right = ParseExpr(opPrece);
		return infixExpr;
	}

	Expr *Parser::ParsePostfixExpr(Expr *prefixExpr)
	{
		auto postfixExpr = new PostfixExpr(GetCurToken());
		postfixExpr->op = GetCurTokenAndStepOnce().literal;
		postfixExpr->left = prefixExpr;
		return postfixExpr;
	}

	Expr *Parser::ParseConditionExpr(Expr *prefixExpr)
	{
		ConditionExpr *conditionExpr = new ConditionExpr(GetCurToken());

		conditionExpr->condition = prefixExpr;
		Consume(TOKEN_QUESTION, L"Expect '?'.");
		conditionExpr->trueBranch = ParseExpr(Precedence::CONDITION);
		Consume(TOKEN_COLON, L"Expect ':' in condition expr");
		conditionExpr->falseBranch = ParseExpr(Precedence::CONDITION);
		return conditionExpr;
	}

	Expr *Parser::ParseIndexExpr(Expr *prefixExpr)
	{
		auto indexExpr = new IndexExpr(GetCurToken());

		Consume(TOKEN_LBRACKET, L"Expect '['.");
		indexExpr->ds = prefixExpr;
		indexExpr->index = ParseExpr();
		Consume(TOKEN_RBRACKET, L"Expect ']'.");
		return indexExpr;
	}

	Expr *Parser::ParseCallExpr(Expr *prefixExpr)
	{
		auto callExpr = new CallExpr(GetCurToken());

		callExpr->callee = prefixExpr;
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
		auto dotExpr = new DotExpr(GetCurToken());
		Consume(TOKEN_DOT, L"Expect '.'.");
		dotExpr->callee = prefixExpr;
		dotExpr->callMember = (IdentifierExpr *)ParseIdentifierExpr();
		return dotExpr;
	}

	Expr *Parser::ParseFactorialExpr(Expr *prefixExpr)
	{
		auto token = Consume(TOKEN_BANG, L"Expect '!'");
		return new FactorialExpr(token, prefixExpr);
	}

	Expr *Parser::ParseVarDescExpr()
	{
		Expr *expr = nullptr;
		if (IsMatchCurToken(TOKEN_ELLIPSIS))
			expr = ParseVarArgExpr();
		else
			expr = (IdentifierExpr *)ParseIdentifierExpr(); // variable name

		// variable type
		std::wstring type = L"any";
		if (IsMatchCurToken(TOKEN_COLON))
		{
			GetCurTokenAndStepOnce();
			type = GetCurTokenAndStepOnce().literal;
		}

		auto varDescExpr = new VarDescExpr(expr->tagToken, type, expr);
		return varDescExpr;
	}

	Expr *Parser::ParseVarArgExpr()
	{
		Consume(TOKEN_ELLIPSIS, L"Expect '...'");

		auto varArgExpr = new VarArgExpr(GetCurToken());

		if (IsMatchCurToken(TOKEN_IDENTIFIER))
		{
			varArgExpr->argName = (IdentifierExpr *)ParseIdentifierExpr();
			return varArgExpr;
		}
		return varArgExpr;
	}

	std::pair<Expr *, Expr *> Parser::ParseDestructuringAssignmentExpr()
	{
		Consume(TOKEN_LBRACKET, L"Expect '['");

		auto arrayExpr = new ArrayExpr(GetCurToken());

		int8_t varArgCount = 0;
		do
		{
			if (IsMatchCurToken(TOKEN_RBRACKET))
				break;

			auto varDescExpr = (VarDescExpr *)ParseVarDescExpr();
			if (varDescExpr->name->type == AST_VAR_ARG) //check if has var arg
			{
				varArgCount++;
				if (varArgCount > 1)
					Hint::Error(varDescExpr->tagToken, L"only 1 variable arg decl is available in var declaration.");
			}

			arrayExpr->elements.emplace_back(varDescExpr);

		} while (IsMatchCurTokenAndStepOnce(TOKEN_COMMA));

		Consume(TOKEN_RBRACKET, L"Expect ']' destructuring assignment expr.");

		if (varArgCount == 1)
		{
			for (int32_t i = 0; i < arrayExpr->elements.size(); ++i)
			{
				if (((VarDescExpr *)arrayExpr->elements[i])->name->type == AST_VAR_ARG)
				{
					if (i < arrayExpr->elements.size() - 1)
						Hint::Error(arrayExpr->elements[i]->tagToken, L"variable arg decl must be located at the end of destructing assignment declaration.");
				}
			}
		}

		ArrayExpr *initializeList = new ArrayExpr(GetCurToken());

		if (IsMatchCurTokenAndStepOnce(TOKEN_EQUAL))
		{
			Expr *value = ParseExpr();

			if (value->type == AST_ARRAY)
			{
				int32_t grad = (int32_t)arrayExpr->elements.size() - (int32_t)((ArrayExpr *)value)->elements.size();
				if (grad == 0)
					return std::make_pair(arrayExpr, value);
				else if (grad > 0)
				{
					initializeList->elements = ((ArrayExpr *)value)->elements;
					for (int32_t i = 0; i < grad; ++i)
						initializeList->elements.emplace_back(new NullExpr(GetCurToken()));
				}
				else if (((VarDescExpr *)arrayExpr->elements.back())->name->type == AST_VAR_ARG)
					initializeList->elements = ((ArrayExpr *)value)->elements;
				else
					Hint::Error(GetCurToken(), L"variable less than value.");
			}
			else if (value->type == AST_CALL)
				return std::make_pair(arrayExpr, value);
			else
				initializeList->elements.resize(arrayExpr->elements.size(), value);
		}
		else
		{
			auto nullExpr = new NullExpr(GetCurToken());
			initializeList->elements.resize(arrayExpr->elements.size(), nullExpr);
		}

		return std::make_pair(arrayExpr, initializeList);
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
		for (const auto &precedenceBinding : precedenceDict)
			if (precedenceBinding.type == GetCurToken().type)
				return precedenceBinding.precedence;
		return Precedence::LOWEST;
	}

	Associativity Parser::GetCurTokenAssociativity()
	{
		for (const auto &associativityBinding : associativityDict)
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
		for (const auto &precedenceBinding : precedenceDict)
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
		if (mSkippingConsumeTokenTypeStack.empty() || type != mSkippingConsumeTokenTypeStack.back())
		{
			if (IsMatchCurToken(type))
				return GetCurTokenAndStepOnce();
			Token token = GetCurToken();
			Hint::Error(token, L"{}", errMsg);
		}
		return Token();
	}

	Token Parser::Consume(const std::vector<TokenType> &types, std::wstring_view errMsg)
	{
		if (!mSkippingConsumeTokenTypeStack.empty())
			for (const auto &type : types)
				if (type == mSkippingConsumeTokenTypeStack.back())
					return Token();

		for (const auto &type : types)
			if (IsMatchCurToken(type))
				return GetCurTokenAndStepOnce();
		Token token = GetCurToken();
		Hint::Error(token, L"{}", errMsg);
	}

	bool Parser::IsAtEnd()
	{
		return mCurPos >= (int32_t)mTokens.size();
	}
}