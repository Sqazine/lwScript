#include "Lexer.h"
namespace lws
{
	struct Keyword
	{
		const wchar_t* name;
		TokenType type;
	};

	constexpr Keyword keywords[] = {
		{L"let", TOKEN_LET},
		{L"if", TOKEN_IF},
		{L"else", TOKEN_ELSE},
		{L"true", TOKEN_TRUE},
		{L"false", TOKEN_FALSE},
		{L"null", TOKEN_NULL},
		{L"while", TOKEN_WHILE},
		{L"for", TOKEN_FOR},
		{L"fn", TOKEN_FUNCTION},
		{L"field", TOKEN_FIELD},
		{L"return", TOKEN_RETURN},
		{L"static", TOKEN_STATIC},
		{L"const", TOKEN_CONST},
		{L"break", TOKEN_BREAK},
		{L"continue", TOKEN_CONTINUE},
	};

	Lexer::Lexer()
	{
		ResetStatus();
	}
	Lexer::~Lexer()
	{
	}

	const std::vector<Token>& Lexer::ScanTokens(std::wstring_view src)
	{
		ResetStatus();
		mSource = src;
		while (!IsAtEnd())
		{
			mStartPos = mCurPos;
			ScanToken();
		}

		AddToken(TOKEN_END, L"EOF");

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
			while (!isascii(GetCurChar()))//not a ASCII char
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
			AddToken(TOKEN_LPAREN);
		else if (c == L")")
			AddToken(TOKEN_RPAREN);
		else if (c == L"[")
			AddToken(TOKEN_LBRACKET);
		else if (c == L"]")
			AddToken(TOKEN_RBRACKET);
		else if (c == L"{")
			AddToken(TOKEN_LBRACE);
		else if (c == L"}")
			AddToken(TOKEN_RBRACE);
		else if (c == L".")
			AddToken(TOKEN_DOT);
		else if (c == L",")
			AddToken(TOKEN_COMMA);
		else if (c == L":")
			AddToken(TOKEN_COLON);
		else if (c == L";")
			AddToken(TOKEN_SEMICOLON);
		else if (c == L"~")
			AddToken(TOKEN_TILDE);
		else if (c == L"?")
			AddToken(TOKEN_QUESTION);
		else if (c == L"\"")
			String();
		else if (c == L" " || c == L"\t" || c == L"\r")
		{

		}
		else if (c == L"\n")
			mLine++;
		else if (c == L"+")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_PLUS_EQUAL);
			else
				AddToken(TOKEN_PLUS);
		}
		else if (c == L"-")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_MINUS_EQUAL);
			else
				AddToken(TOKEN_MINUS);
		}
		else if (c == L"*")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_ASTERISK_EQUAL);
			else
				AddToken(TOKEN_ASTERISK);
		}
		else if (c == L"/")
		{
			if (IsMatchCurCharAndStepOnce('/'))
			{
				while (!IsMatchCurChar('\n') && !IsAtEnd())
					GetCurCharAndStepOnce();
				mLine++;
			}
			else if (IsMatchCurCharAndStepOnce('*'))
			{
				while (!IsMatchCurChar('*') && !IsMatchNextChar('/') && !IsAtEnd())
				{
					if (IsMatchCurChar('\n'))
						mLine++;
					GetCurCharAndStepOnce();
				}
				GetCurCharAndStepOnce(); //eat '*'
				GetCurCharAndStepOnce(); // eat '/'
			}
			else if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_SLASH_EQUAL);
			else
				AddToken(TOKEN_SLASH);
		}
		else if (c == L"%")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_PERCENT_EQUAL);
			AddToken(TOKEN_PERCENT);
		}
		else if (c == L"!")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_BANG_EQUAL);
			else
				AddToken(TOKEN_BANG);
		}
		else if (c == L"&")
		{
			if (IsMatchCurCharAndStepOnce('&'))
				AddToken(TOKEN_AMPERSAND_AMPERSAND);
			else if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_AMPERSAND_EQUAL);
			else
				AddToken(TOKEN_AMPERSAND);
		}
		else if (c == L"|")
		{
			if (IsMatchCurCharAndStepOnce('|'))
				AddToken(TOKEN_VBAR_VBAR);
			else if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_VBAR_EQUAL);
			else
				AddToken(TOKEN_VBAR);
		}
		else if (c == L"^")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_CARET_EQUAL);
			else
				AddToken(TOKEN_CARET);
		}
		else if (c == L"<")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_LESS_EQUAL);
			else if (IsMatchCurCharAndStepOnce('<'))
			{
				if (IsMatchCurCharAndStepOnce('='))
					AddToken(TOKEN_LESS_LESS_EQUAL);
				else
					AddToken(TOKEN_LESS_LESS);
			}
			else
				AddToken(TOKEN_LESS);
		}
		else if (c == L">")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_GREATER_EQUAL);
			else if (IsMatchCurCharAndStepOnce('>'))
			{
				if (IsMatchCurCharAndStepOnce('='))
					AddToken(TOKEN_GREATER_GREATER_EQUAL);
				else
					AddToken(TOKEN_GREATER_GREATER);
			}
			else
				AddToken(TOKEN_GREATER);
		}
		else if (c == L"=")
		{
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TOKEN_EQUAL_EQUAL);
			else
				AddToken(TOKEN_EQUAL);
		}
		else
		{
			if (IsNumber(ch))
				Number();
			else if (IsLetter(ch,isAscii))
				Identifier();
			else
			{
				auto literal = mSource.substr(mStartPos, mCurPos - mStartPos);
				Assert(L"Unknown literal:" + literal);
			}
		}

	}

	void Lexer::ResetStatus()
	{
		mStartPos = mCurPos = 0;
		mLine = 1;
		std::vector<Token>().swap(mTokens);
	}

	bool Lexer::IsMatchCurChar(char c)
	{
		return GetCurChar() == c;
	}
	bool Lexer::IsMatchCurCharAndStepOnce(char c)
	{
		bool result = GetCurChar() == c;
		if (result)
			mCurPos++;
		return result;
	}

	bool Lexer::IsMatchNextChar(char c)
	{
		return GetNextChar() == c;
	}
	bool Lexer::IsMatchNextCharAndStepOnce(char c)
	{
		bool result = GetNextChar() == c;
		if (result)
			mCurPos++;
		return result;
	}

	wchar_t Lexer::GetNextCharAndStepOnce()
	{
		if (mCurPos + 1 < mSource.size())
			return mSource[++mCurPos];
		return '\0';
	}
	wchar_t Lexer::GetNextChar()
	{
		if (mCurPos + 1 < mSource.size())
			return mSource[mCurPos + 1];
		return '\0';
	}
	wchar_t Lexer::GetCurCharAndStepOnce()
	{
		if (!IsAtEnd())
			return mSource[mCurPos++];
		return '\0';
	}

	wchar_t Lexer::GetCurChar()
	{
		if (!IsAtEnd())
			return mSource[mCurPos];
		return '\0';
	}

	void Lexer::AddToken(TokenType type)
	{
		auto literal = mSource.substr(mStartPos, mCurPos - mStartPos);
		mTokens.emplace_back(Token(type, literal, mLine));
	}
	void Lexer::AddToken(TokenType type, std::wstring_view literal)
	{
		mTokens.emplace_back(Token(type, literal, mLine));
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
		return IsLetter(c,isAscii) || IsNumber(c);
	}

	void Lexer::Number()
	{
		while (IsNumber(GetCurChar()))
			GetCurCharAndStepOnce();

		if (IsMatchCurCharAndStepOnce('.'))
		{
			if (IsNumber(GetCurChar()))
				while (IsNumber(GetCurChar()))
					GetCurCharAndStepOnce();
			else
				Assert(L"[line " + std::to_wstring(mLine) + L"]:Number cannot end with '.'");
		}

		AddToken(TOKEN_NUMBER);
	}

	void Lexer::Identifier()
	{
		wchar_t c = GetCurChar();
		bool isAscii = isascii(c)?true:false;
		while (IsLetterOrNumber(c, isAscii))
		{
			GetCurCharAndStepOnce();
			c = GetCurChar();
			isAscii = isascii(c) ? true : false;
		}

		std::wstring literal = mSource.substr(mStartPos, mCurPos - mStartPos);

		bool isKeyWord = false;
		for (const auto& keyword : keywords)
			if (wcscmp(keyword.name, literal.c_str()) == 0)
			{
				AddToken(keyword.type, literal);
				isKeyWord = true;
				break;
			}

		if (!isKeyWord)
			AddToken(TOKEN_IDENTIFIER, literal);
	}

	void Lexer::String()
	{
		while (!IsMatchCurChar('\"') && !IsAtEnd())
		{
			if (IsMatchCurChar('\n'))
				mLine++;
			GetCurCharAndStepOnce();
		}

		if (IsAtEnd())
			std::wcout << "[line " << mLine << "]:Uniterminated string." << std::endl;

		GetCurCharAndStepOnce(); //eat the second '\"'

		AddToken(TOKEN_STRING, mSource.substr(mStartPos + 1, mCurPos - mStartPos - 2));
	}
}