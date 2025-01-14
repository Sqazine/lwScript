#include "Lexer.h"
#include "Logger.h"
namespace lwscript
{
	constexpr struct
	{
		const CHAR_T *name;
		TokenKind type;
	} keywords[] = {
		{TEXT("let"), TokenKind::LET},
		{TEXT("if"), TokenKind::IF},
		{TEXT("else"), TokenKind::ELSE},
		{TEXT("true"), TokenKind::TRUE},
		{TEXT("false"), TokenKind::FALSE},
		{TEXT("null"), TokenKind::NIL},
		{TEXT("while"), TokenKind::WHILE},
		{TEXT("for"), TokenKind::FOR},
		{TEXT("fn"), TokenKind::FUNCTION},
		{TEXT("class"), TokenKind::CLASS},
		{TEXT("this"), TokenKind::THIS},
		{TEXT("base"), TokenKind::BASE},
		{TEXT("return"), TokenKind::RETURN},
		{TEXT("static"), TokenKind::STATIC},
		{TEXT("const"), TokenKind::CONST},
		{TEXT("break"), TokenKind::BREAK},
		{TEXT("continue"), TokenKind::CONTINUE},
		{TEXT("import"), TokenKind::IMPORT},
		{TEXT("module"), TokenKind::MODULE},
		{TEXT("switch"), TokenKind::SWITCH},
		{TEXT("default"), TokenKind::DEFAULT},
		{TEXT("match"), TokenKind::MATCH},
		{TEXT("enum"), TokenKind::ENUM},
		{TEXT("u8"), TokenKind::U8},
		{TEXT("u16"), TokenKind::U16},
		{TEXT("u32"), TokenKind::U32},
		{TEXT("u64"), TokenKind::U64},
		{TEXT("i8"), TokenKind::I8},
		{TEXT("i16"), TokenKind::I16},
		{TEXT("i32"), TokenKind::I32},
		{TEXT("i64"), TokenKind::I64},
		{TEXT("f32"), TokenKind::F32},
		{TEXT("f64"), TokenKind::F64},
		{TEXT("bool"), TokenKind::BOOL},
		{TEXT("char"), TokenKind::CHAR},
		{TEXT("void"), TokenKind::VOID},
		{TEXT("any"), TokenKind::ANY},
		{TEXT("as"), TokenKind::AS},
		{TEXT("new"), TokenKind::NEW},
		{TEXT("struct"), TokenKind::STRUCT},
	};

	Lexer::Lexer()
	{
		ResetStatus();
	}

	const std::vector<Token *> &Lexer::ScanTokens(STD_STRING_VIEW src)
	{
		Logger::RecordSource(src);

		ResetStatus();
		mSource = src;
		while (!IsAtEnd())
		{
			mStartPos = mCurPos;
			ScanToken();
		}

		AddToken(TokenKind::END, TEXT("END"));

		return mTokens;
	}
	void Lexer::ScanToken()
	{
		STD_STRING c;

		CHAR_T ch;
		bool isAscii = true;
		if (!isascii(GetCurChar()))
		{
			ch = GetCurChar();
			isAscii = false;
			while (!isascii(GetCurChar())) // not a ASCII char
			{
				c.append(1, ch);
				ch = GetCurCharAndStepOnce();
			}
		}
		else
		{
			ch = GetCurCharAndStepOnce();
			c.append(1, ch);
		}

		if (c == TEXT("("))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('{')))
				AddToken(TokenKind::LPAREN_LBRACE);
			else
				AddToken(TokenKind::LPAREN);
		}
		else if (c == TEXT(")"))
			AddToken(TokenKind::RPAREN);
		else if (c == TEXT("["))
			AddToken(TokenKind::LBRACKET);
		else if (c == TEXT("]"))
			AddToken(TokenKind::RBRACKET);
		else if (c == TEXT("{"))
			AddToken(TokenKind::LBRACE);
		else if (c == TEXT("}"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR(')')))
				AddToken(TokenKind::RBRACE_RPAREN);
			else
				AddToken(TokenKind::RBRACE);
		}
		else if (c == TEXT("."))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('.')))
			{
				if (IsMatchCurCharAndStepOnce(TCHAR('.')))
					AddToken(TokenKind::ELLIPSIS);
				else
					Logger::Error(mCurPos, TEXT("Unknown literal:'..',did you want '.' or '...'?"));
			}
			else
				AddToken(TokenKind::DOT);
		}
		else if (c == TEXT(","))
			AddToken(TokenKind::COMMA);
		else if (c == TEXT(":"))
			AddToken(TokenKind::COLON);
		else if (c == TEXT(";"))
			AddToken(TokenKind::SEMICOLON);
		else if (c == TEXT("~"))
			AddToken(TokenKind::TILDE);
		else if (c == TEXT("?"))
			AddToken(TokenKind::QUESTION);
		else if (c == TEXT("\""))
			String();
		else if (c == TEXT("\'"))
			Character();
		else if (c == TEXT(" ") || c == TEXT("\t") || c == TEXT("\r"))
		{
		}
		else if (c == TEXT("\n"))
		{
			mLine++;
			mColumn = 1;
		}
		else if (c == TEXT("+"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::PLUS_EQUAL);
			else if (IsMatchCurCharAndStepOnce(TCHAR('+')))
				AddToken(TokenKind::PLUS_PLUS);
			else
				AddToken(TokenKind::PLUS);
		}
		else if (c == TEXT("-"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::MINUS_EQUAL);
			else if (IsMatchCurCharAndStepOnce(TCHAR('-')))
				AddToken(TokenKind::MINUS_MINUS);
			else
				AddToken(TokenKind::MINUS);
		}
		else if (c == TEXT("*"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::ASTERISK_EQUAL);
			else
				AddToken(TokenKind::ASTERISK);
		}
		else if (c == TEXT("/"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('/')))
			{
				while (!IsMatchCurChar(TCHAR('\n')) && !IsAtEnd())
					GetCurCharAndStepOnce();
			}
			else if (IsMatchCurCharAndStepOnce(TCHAR('*')))
			{
				while (!IsAtEnd())
				{
					if (IsMatchCurChar(TCHAR('\n')))
					{
						mLine++;
						mColumn = 1;
					}
					Logger::Println(TEXT("{}"), GetCurChar());
					GetCurCharAndStepOnce();
					if (IsMatchCurChar(TCHAR('*')))
					{
						GetCurCharAndStepOnce();
						if (IsMatchCurChar(TCHAR('/')))
						{
							GetCurCharAndStepOnce();
							break;
						}
					}
				}
			}
			else if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::SLASH_EQUAL);
			else
				AddToken(TokenKind::SLASH);
		}
		else if (c == TEXT("%"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::PERCENT_EQUAL);
			AddToken(TokenKind::PERCENT);
		}
		else if (c == TEXT("!"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::BANG_EQUAL);
			else
				AddToken(TokenKind::BANG);
		}
		else if (c == TEXT("&"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('&')))
				AddToken(TokenKind::AMPERSAND_AMPERSAND);
			else if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::AMPERSAND_EQUAL);
			else
				AddToken(TokenKind::AMPERSAND);
		}
		else if (c == TEXT("|"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('|')))
				AddToken(TokenKind::VBAR_VBAR);
			else if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::VBAR_EQUAL);
			else
				AddToken(TokenKind::VBAR);
		}
		else if (c == TEXT("^"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::CARET_EQUAL);
			else
				AddToken(TokenKind::CARET);
		}
		else if (c == TEXT("<"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::LESS_EQUAL);
			else if (IsMatchCurCharAndStepOnce(TCHAR('<')))
			{
				if (IsMatchCurCharAndStepOnce(TCHAR('=')))
					AddToken(TokenKind::LESS_LESS_EQUAL);
				else
					AddToken(TokenKind::LESS_LESS);
			}
			else
				AddToken(TokenKind::LESS);
		}
		else if (c == TEXT(">"))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::GREATER_EQUAL);
			else if (IsMatchCurCharAndStepOnce(TCHAR('>')))
			{
				if (IsMatchCurCharAndStepOnce(TCHAR('=')))
					AddToken(TokenKind::GREATER_GREATER_EQUAL);
				else
					AddToken(TokenKind::GREATER_GREATER);
			}
			else
				AddToken(TokenKind::GREATER);
		}
		else if (c == TEXT("="))
		{
			if (IsMatchCurCharAndStepOnce(TCHAR('=')))
				AddToken(TokenKind::EQUAL_EQUAL);
			else
				AddToken(TokenKind::EQUAL);
		}
		else
		{
			if (IsNumber(ch))
				Number();
			else if (IsLetter(ch, isAscii))
				Identifier();
			else
			{
				auto literal = mSource.substr(mStartPos, mCurPos - mStartPos);
				Logger::Error(mCurPos, TEXT("Unknown literal:") + literal);
			}
		}
	}

	void Lexer::ResetStatus()
	{
		mStartPos = mCurPos = 0;
		mLine = 1;
		mColumn = 1;
		std::vector<Token *>().swap(mTokens);
	}

	bool Lexer::IsMatchCurChar(CHAR_T c)
	{
		return GetCurChar() == c;
	}
	bool Lexer::IsMatchCurCharAndStepOnce(CHAR_T c)
	{
		bool result = GetCurChar() == c;
		if (result)
		{
			mCurPos++;
			mColumn++;
		}
		return result;
	}

	CHAR_T Lexer::GetCurCharAndStepOnce()
	{
		if (!IsAtEnd())
		{
			mColumn++;
			return mSource[mCurPos++];
		}
		return TCHAR('\0');
	}

	CHAR_T Lexer::GetCurChar()
	{
		if (!IsAtEnd())
			return mSource[mCurPos];
		return TCHAR('\0');
	}

	void Lexer::AddToken(TokenKind type)
	{
		auto literal = mSource.substr(mStartPos, mCurPos - mStartPos);

		SourceLocation srcLoc;
		srcLoc.line = mLine;
		srcLoc.column = mColumn - literal.size();
		srcLoc.pos = mCurPos - literal.size();

		mTokens.push_back(new Token(type, literal, srcLoc));
	}
	void Lexer::AddToken(TokenKind type, STD_STRING_VIEW literal)
	{
		SourceLocation srcLoc;
		srcLoc.line = mLine;
		srcLoc.column = mColumn - literal.size();
		srcLoc.pos = mCurPos - literal.size();
		mTokens.push_back(new Token(type, literal, srcLoc));
	}

	bool Lexer::IsAtEnd()
	{
		return mCurPos >= mSource.size();
	}

	bool Lexer::IsNumber(CHAR_T c)
	{
		return c >= TCHAR('0') && c <= TCHAR('9');
	}
	bool Lexer::IsLetter(CHAR_T c, bool isAscii)
	{
		if (!isAscii)
			return true;
		return (c >= TCHAR('A') && c <= TCHAR('Z')) || (c >= TCHAR('a') && c <= TCHAR('z')) || c == TCHAR('_');
	}
	bool Lexer::IsLetterOrNumber(CHAR_T c, bool isAscii)
	{
		return IsLetter(c, isAscii) || IsNumber(c);
	}

	void Lexer::Number()
	{
		while (IsNumber(GetCurChar()))
			GetCurCharAndStepOnce();

		if (IsMatchCurCharAndStepOnce(TCHAR('.')))
		{
			if (IsNumber(GetCurChar()))
				while (IsNumber(GetCurChar()))
					GetCurCharAndStepOnce();
			else if (GetCurChar() == TCHAR('f'))
				GetCurCharAndStepOnce();
			else
				Logger::Error(mCurPos, TEXT("The character next to '.' in a floating number must be in [0-9] range or a single 'f' character."));
		}

		AddToken(TokenKind::NUMBER);
	}

	void Lexer::Identifier()
	{
		CHAR_T c = GetCurChar();
		bool isAscii = isascii(c) ? true : false;
		while (IsLetterOrNumber(c, isAscii))
		{
			GetCurCharAndStepOnce();
			c = GetCurChar();
			isAscii = isascii(c) ? true : false;
		}

		STD_STRING literal = mSource.substr(mStartPos, mCurPos - mStartPos);

		bool isKeyWord = false;
		for (const auto &keyword : keywords)
			if (STRCMP(keyword.name, literal.c_str()) == 0)
			{
				AddToken(keyword.type, literal);
				isKeyWord = true;
				break;
			}

		if (!isKeyWord)
			AddToken(TokenKind::IDENTIFIER, literal);
	}

	void Lexer::String()
	{
		while (!IsMatchCurChar(TCHAR('\"')) && !IsAtEnd())
		{
			if (IsMatchCurChar(TCHAR('\n')))
			{
				mLine++;
				mColumn = 1;
			}
			GetCurCharAndStepOnce();
		}

		if (IsAtEnd())
			Logger::Println(TEXT("[line {}]:Uniterminated string."), mLine);

		GetCurCharAndStepOnce(); // eat the second '\"'

		AddToken(TokenKind::STRING, mSource.substr(mStartPos + 1, mCurPos - mStartPos - 2));
	}

	void Lexer::Character()
	{
		GetCurCharAndStepOnce(); // eat the first '\''

		AddToken(TokenKind::CHARACTER, mSource.substr(mStartPos + 1, 1));

		GetCurCharAndStepOnce(); // eat the second '\''
	}
}