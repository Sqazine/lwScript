#pragma once
#include <cstdint>
#include <vector>
#include <string>

#include "Token.h"
namespace lwScript
{
    class Lexer
    {
    public:
        Lexer();
        ~Lexer();

       const std::vector<Token>& ScanTokens(std::string_view src);
    private:
        void ResetStatus();

        void ScanToken();

        bool IsMatchCurChar(char c);
        bool IsMatchCurCharAndStepOnce(char c);
        bool IsMatchNextChar(char c);
        bool IsMatchNextCharAndStepOnce(char c);

        char GetNextCharAndStepOnce();
        char GetNextChar();
        char GetCurCharAndStepOnce();
        char GetCurChar();

        void AddToken(TokenType type);
        void AddToken(TokenType type,std::string_view literal);

        bool IsAtEnd();

        bool IsNumber(char c);
        bool IsLetter(char c);
        bool IsLetterOrNumber(char c);

        void Number();
        void Identifier();
        void String();

        uint64_t m_StartPos;
        uint64_t m_CurPos;
        uint64_t m_Line;
        std::string m_Source;
        std::vector<Token> m_Tokens;
    };


    static std::unordered_map<std::string, TokenType> keywords =
	{
		{"let", TokenType::LET},
		{"if",  TokenType::IF},
		{"else",TokenType::ELSE},
		{"true", TokenType::TRUE},
		{"false", TokenType::FALSE},
		{"nil", TokenType::NIL},
		{"while", TokenType::WHILE},
		{"fn", TokenType::FUNCTION},
		{"struct",TokenType::STRUCT},
		{"return",TokenType::RETURN},
		{"ref",TokenType::REF}
	};

	Lexer::Lexer()
	{
		ResetStatus();
	}
	Lexer::~Lexer()
	{
	}

	const std::vector<Token>& Lexer::ScanTokens(std::string_view src)
	{
		ResetStatus();
		m_Source = src;
		while (!IsAtEnd())
		{
			m_StartPos = m_CurPos;
			ScanToken();
		}

		AddToken(TokenType::END, "EOF");

		return m_Tokens;
	}
	void Lexer::ScanToken()
	{
		char c = GetCurCharAndStepOnce();

		switch (c)
		{
		case '(':
			AddToken(TokenType::LPAREN);
			break;
		case ')':
			AddToken(TokenType::RPAREN);
			break;
		case '[':
			AddToken(TokenType::LBRACKET);
			break;
		case ']':
			AddToken(TokenType::RBRACKET);
			break;
		case '{':
			AddToken(TokenType::LBRACE);
			break;
		case '}':
			AddToken(TokenType::RBRACE);
			break;
		case ',':
			AddToken(TokenType::COMMA);
			break;
		case '.':
			AddToken(TokenType::DOT);
			break;
		case ':':
			AddToken(TokenType::COLON);
			break;
		case ';':
			AddToken(TokenType::SEMICOLON);
			break;
		case '#':
			AddToken(TokenType::SHARP);
			break;
		case '\"':
			String();
			break;
		case ' ':
		case '\t':
		case '\r':
			break;
		case '\n':
			m_Line++;
			break;
		case '+':
				AddToken(TokenType::PLUS);
			break;
		case '-':
				AddToken(TokenType::MINUS);
			break;
		case '*':
				AddToken(TokenType::ASTERISK);
			break;
		case '/':
				AddToken(TokenType::SLASH);
			break;
		case '!':
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TokenType::BEQUAL);
			else
				AddToken(TokenType::UNKNOWN);
			break;
		case '&':
			if (IsMatchCurCharAndStepOnce('&'))
				AddToken(TokenType::AND);
			else
				AddToken(TokenType::UNKNOWN);
			break;
		case '|':
			if (IsMatchCurCharAndStepOnce('|'))
				AddToken(TokenType::OR);
			else
				AddToken(TokenType::UNKNOWN);
			break;
		case '<':
			 if (IsMatchCurCharAndStepOnce('='))
				AddToken(TokenType::LEQUAL);
			else
				AddToken(TokenType::LESS);
			break;
		case '>':
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TokenType::GEQUAL);
			else
				AddToken(TokenType::GREATER);
			break;
		case '=':
			if (IsMatchCurCharAndStepOnce('='))
				AddToken(TokenType::EEQUAL);
			else
				AddToken(TokenType::EQUAL);
			break;
		default:
			if (IsNumber(c))
				Number();
			else if (IsLetter(c))
				Identifier();
			else
				AddToken(TokenType::UNKNOWN);
			break;
		}
	}

	void Lexer::ResetStatus()
	{
		m_StartPos = m_CurPos = 0;
		m_Line = 1;
		std::vector<Token>().swap(m_Tokens);
	}

	bool Lexer::IsMatchCurChar(char c)
	{
		return GetCurChar() == c;
	}
	bool Lexer::IsMatchCurCharAndStepOnce(char c)
	{
		bool result = GetCurChar() == c;
		if (result)
			m_CurPos++;
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
			m_CurPos++;
		return result;
	}

	char Lexer::GetNextCharAndStepOnce()
	{
		if (m_CurPos + 1 < m_Source.size())
			return m_Source[++m_CurPos];
		return '\0';
	}
	char Lexer::GetNextChar()
	{
		if (m_CurPos + 1 < m_Source.size())
			return m_Source[m_CurPos + 1];
		return '\0';
	}
	char Lexer::GetCurCharAndStepOnce()
	{
		if (!IsAtEnd())
			return m_Source[m_CurPos++];
		return '\0';
	}

	char Lexer::GetCurChar()
	{
		if (!IsAtEnd())
			return m_Source[m_CurPos];
		return '\0';
	}

	void Lexer::AddToken(TokenType type)
	{
		auto literal = m_Source.substr(m_StartPos, m_CurPos - m_StartPos);
		m_Tokens.emplace_back(Token(type, literal, m_Line));
	}
	void Lexer::AddToken(TokenType type, std::string_view literal)
	{
		m_Tokens.emplace_back(Token(type, literal, m_Line));
	}

	bool Lexer::IsAtEnd()
	{
		return m_CurPos >= m_Source.size();
	}

	bool Lexer::IsNumber(char c)
	{
		return c >= '0' && c <= '9';
	}
	bool Lexer::IsLetter(char c)
	{
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
	}
	bool Lexer::IsLetterOrNumber(char c)
	{
		return IsLetter(c) || IsNumber(c);
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
			{
				std::cout << "[line "<<m_Line<<"]:Number cannot end with '.'" << std::endl;
				exit(1);
			}
		}

		AddToken(TokenType::NUMBER);
	}

	void Lexer::Identifier()
	{
		while (IsLetterOrNumber(GetCurChar()))
			GetCurCharAndStepOnce();

		std::string literal = m_Source.substr(m_StartPos, m_CurPos - m_StartPos);

		bool isKeyWord = false;
		for (const auto& [key, value] : keywords)
			if (key.compare(literal) == 0)
			{
				AddToken(value, literal);
				isKeyWord = true;
				break;
			}

		if (!isKeyWord)
			AddToken(TokenType::IDENTIFIER, literal);
	}

	void Lexer::String()
	{
		while (!IsMatchCurChar('\"') && !IsAtEnd())
		{
			if (IsMatchCurChar('\n'))
				m_Line++;
			GetCurCharAndStepOnce();
		}

		if (IsAtEnd())
			std::cout << "[line " << m_Line << "]:Uniterminated string." << std::endl;

		GetCurCharAndStepOnce(); //eat the second '\"'

		AddToken(TokenType::STRING, m_Source.substr(m_StartPos + 1, m_CurPos - m_StartPos - 2));
	}

}