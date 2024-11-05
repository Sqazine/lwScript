#include "Lexer.h"
#include "Logger.h"
namespace lwscript
{
	struct Keyword
	{
		const wchar_t *name;
		TokenKind type;
	};

	constexpr Keyword keywords[] = {
		{L"let", TokenKind::LET},
		{L"if", TokenKind::IF},
		{L"else", TokenKind::ELSE},
		{L"true", TokenKind::TRUE},
		{L"false", TokenKind::FALSE},
		{L"null", TokenKind::NIL},
		{L"while", TokenKind::WHILE},
		{L"for", TokenKind::FOR},
		{L"fn", TokenKind::FUNCTION},
		{L"class", TokenKind::CLASS},
		{L"this", TokenKind::THIS},
		{L"base", TokenKind::BASE},
		{L"return", TokenKind::RETURN},
		{L"static", TokenKind::STATIC},
		{L"const", TokenKind::CONST},
		{L"break", TokenKind::BREAK},
		{L"continue", TokenKind::CONTINUE},
		{L"import", TokenKind::IMPORT},
		{L"module", TokenKind::MODULE},
		{L"switch", TokenKind::SWITCH},
		{L"default", TokenKind::DEFAULT},
		{L"match", TokenKind::MATCH},
		{L"enum", TokenKind::ENUM},
		{L"u8", TokenKind::U8},
		{L"u16", TokenKind::U16},
		{L"u32", TokenKind::U32},
		{L"u64", TokenKind::U64},
		{L"i8", TokenKind::I8},
		{L"i16", TokenKind::I16},
		{L"i32", TokenKind::I32},
		{L"i64", TokenKind::I64},
		{L"bool", TokenKind::BOOL},
		{L"char", TokenKind::CHAR},
		{L"void", TokenKind::VOID},
		{L"as", TokenKind::AS},
		{L"new", TokenKind::NEW},
		{L"struct", TokenKind::STRUCT},
	};

	Lexer::Lexer()
	{
		ResetStatus();
	}
	Lexer::~Lexer()
	{
	}

	const std::vector<Token *> &Lexer::ScanTokens(std::wstring_view src)
	{
		Logger::RecordSource(src);

		ResetStatus();
		mSource = src;
		while (!IsAtEnd())
		{
			mStartPos = mCurPos;
			ScanToken();
		}

		AddToken(TokenKind::END, L"EOF");

		return mTokens;
	}
	void Lexer::ScanToken()
	{
		std::wstring c;

		wchar_t ch;
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

		if (c == L"(")
		{
			if (IsMatchCurCharAndStepOnce(L'{'))
				AddToken(TokenKind::LPAREN_LBRACE);
			else
				AddToken(TokenKind::LPAREN);
		}
		else if (c == L")")
			AddToken(TokenKind::RPAREN);
		else if (c == L"[")
			AddToken(TokenKind::LBRACKET);
		else if (c == L"]")
			AddToken(TokenKind::RBRACKET);
		else if (c == L"{")
			AddToken(TokenKind::LBRACE);
		else if (c == L"}")
		{
			if (IsMatchCurCharAndStepOnce(L')'))
				AddToken(TokenKind::RBRACE_RPAREN);
			else
				AddToken(TokenKind::RBRACE);
		}
		else if (c == L".")
		{
			if (IsMatchCurCharAndStepOnce(L'.'))
			{
				if (IsMatchCurCharAndStepOnce(L'.'))
					AddToken(TokenKind::ELLIPSIS);
				else
					Logger::Error(mCurPos, L"Unknown literal:'..',did you want '.' or '...'?");
			}
			else
				AddToken(TokenKind::DOT);
		}
		else if (c == L",")
			AddToken(TokenKind::COMMA);
		else if (c == L":")
			AddToken(TokenKind::COLON);
		else if (c == L";")
			AddToken(TokenKind::SEMICOLON);
		else if (c == L"~")
			AddToken(TokenKind::TILDE);
		else if (c == L"?")
			AddToken(TokenKind::QUESTION);
		else if (c == L"\"")
			String();
		else if (c == L"\'")
			Character();
		else if (c == L" " || c == L"\t" || c == L"\r")
		{
		}
		else if (c == L"\n")
		{
			mLine++;
			mColumn = 1;
		}
		else if (c == L"+")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::PLUS_EQUAL);
			else if (IsMatchCurCharAndStepOnce(L'+'))
				AddToken(TokenKind::PLUS_PLUS);
			else
				AddToken(TokenKind::PLUS);
		}
		else if (c == L"-")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::MINUS_EQUAL);
			else if (IsMatchCurCharAndStepOnce(L'-'))
				AddToken(TokenKind::MINUS_MINUS);
			else
				AddToken(TokenKind::MINUS);
		}
		else if (c == L"*")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::ASTERISK_EQUAL);
			else
				AddToken(TokenKind::ASTERISK);
		}
		else if (c == L"/")
		{
			if (IsMatchCurCharAndStepOnce(L'/'))
			{
				while (!IsMatchCurChar('\n') && !IsAtEnd())
					GetCurCharAndStepOnce();
			}
			else if (IsMatchCurCharAndStepOnce(L'*'))
			{
				while (!IsAtEnd())
				{
					if (IsMatchCurChar(L'\n'))
					{
						mLine++;
						mColumn = 1;
					}
					Println(L"{}", GetCurChar());
					GetCurCharAndStepOnce();
					if (IsMatchCurChar(L'*'))
					{
						GetCurCharAndStepOnce();
						if (IsMatchCurChar(L'/'))
						{
							GetCurCharAndStepOnce();
							break;
						}
					}
				}
			}
			else if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::SLASH_EQUAL);
			else
				AddToken(TokenKind::SLASH);
		}
		else if (c == L"%")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::PERCENT_EQUAL);
			AddToken(TokenKind::PERCENT);
		}
		else if (c == L"!")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::BANG_EQUAL);
			else
				AddToken(TokenKind::BANG);
		}
		else if (c == L"&")
		{
			if (IsMatchCurCharAndStepOnce(L'&'))
				AddToken(TokenKind::AMPERSAND_AMPERSAND);
			else if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::AMPERSAND_EQUAL);
			else
				AddToken(TokenKind::AMPERSAND);
		}
		else if (c == L"|")
		{
			if (IsMatchCurCharAndStepOnce(L'|'))
				AddToken(TokenKind::VBAR_VBAR);
			else if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::VBAR_EQUAL);
			else
				AddToken(TokenKind::VBAR);
		}
		else if (c == L"^")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::CARET_EQUAL);
			else
				AddToken(TokenKind::CARET);
		}
		else if (c == L"<")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::LESS_EQUAL);
			else if (IsMatchCurCharAndStepOnce(L'<'))
			{
				if (IsMatchCurCharAndStepOnce(L'='))
					AddToken(TokenKind::LESS_LESS_EQUAL);
				else
					AddToken(TokenKind::LESS_LESS);
			}
			else
				AddToken(TokenKind::LESS);
		}
		else if (c == L">")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
				AddToken(TokenKind::GREATER_EQUAL);
			else if (IsMatchCurCharAndStepOnce(L'>'))
			{
				if (IsMatchCurCharAndStepOnce(L'='))
					AddToken(TokenKind::GREATER_GREATER_EQUAL);
				else
					AddToken(TokenKind::GREATER_GREATER);
			}
			else
				AddToken(TokenKind::GREATER);
		}
		else if (c == L"=")
		{
			if (IsMatchCurCharAndStepOnce(L'='))
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
				Logger::Error(mCurPos, L"Unknown literal:" + literal);
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

	bool Lexer::IsMatchCurChar(wchar_t c)
	{
		return GetCurChar() == c;
	}
	bool Lexer::IsMatchCurCharAndStepOnce(wchar_t c)
	{
		bool result = GetCurChar() == c;
		if (result)
		{
			mCurPos++;
			mColumn++;
		}
		return result;
	}

	wchar_t Lexer::GetCurCharAndStepOnce()
	{
		if (!IsAtEnd())
		{
			mColumn++;
			return mSource[mCurPos++];
		}
		return L'\0';
	}

	wchar_t Lexer::GetCurChar()
	{
		if (!IsAtEnd())
			return mSource[mCurPos];
		return L'\0';
	}

	void Lexer::AddToken(TokenKind type)
	{
		auto literal = mSource.substr(mStartPos, mCurPos - mStartPos);
		mTokens.push_back(new Token(type, literal, mLine, mColumn - literal.size(), mCurPos - literal.size())); //+1 means that the column beginning front 1
	}
	void Lexer::AddToken(TokenKind type, std::wstring_view literal)
	{
		mTokens.push_back(new Token(type, literal, mLine, mColumn - literal.size(), mCurPos - literal.size()));
	}

	bool Lexer::IsAtEnd()
	{
		return mCurPos >= mSource.size();
	}

	bool Lexer::IsNumber(wchar_t c)
	{
		return c >= L'0' && c <= L'9';
	}
	bool Lexer::IsLetter(wchar_t c, bool isAscii)
	{
		if (!isAscii)
			return true;
		return (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z') || c == L'_';
	}
	bool Lexer::IsLetterOrNumber(wchar_t c, bool isAscii)
	{
		return IsLetter(c, isAscii) || IsNumber(c);
	}

	void Lexer::Number()
	{
		while (IsNumber(GetCurChar()))
			GetCurCharAndStepOnce();

		if (IsMatchCurCharAndStepOnce(L'.'))
		{
			if (IsNumber(GetCurChar()))
				while (IsNumber(GetCurChar()))
					GetCurCharAndStepOnce();
			else if (GetCurChar() == L'f')
				GetCurCharAndStepOnce();
			else
				Logger::Error(mCurPos, L"The character next to '.' in a floating number must be in [0-9] range or a single 'f' character.");
		}

		AddToken(TokenKind::NUMBER);
	}

	void Lexer::Identifier()
	{
		wchar_t c = GetCurChar();
		bool isAscii = isascii(c) ? true : false;
		while (IsLetterOrNumber(c, isAscii))
		{
			GetCurCharAndStepOnce();
			c = GetCurChar();
			isAscii = isascii(c) ? true : false;
		}

		std::wstring literal = mSource.substr(mStartPos, mCurPos - mStartPos);

		bool isKeyWord = false;
		for (const auto &keyword : keywords)
			if (wcscmp(keyword.name, literal.c_str()) == 0)
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
		while (!IsMatchCurChar(L'\"') && !IsAtEnd())
		{
			if (IsMatchCurChar(L'\n'))
			{
				mLine++;
				mColumn = 1;
			}
			GetCurCharAndStepOnce();
		}

		if (IsAtEnd())
			Println(L"[line {}]:Uniterminated string.", mLine);

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