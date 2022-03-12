#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Utils.h"
namespace lws
{
	class Lexer
	{
	public:
		Lexer();
		~Lexer();

		const std::vector<Token> &GenerateTokens(std::string_view src);

	private:
		void ResetStatus();

		void GenerateToken();

		bool IsMatchCurChar(char c);
		bool IsMatchCurCharAndStepOnce(char c);
		bool IsMatchNextChar(char c);
		bool IsMatchNextCharAndStepOnce(char c);

		char GetNextCharAndStepOnce();
		char GetNextChar();
		char GetCurCharAndStepOnce();
		char GetCurChar();

		void AddToken(TokenType type);
		void AddToken(TokenType type, std::string_view literal);

		bool IsAtEnd();

		bool IsNumber(char c);
		bool IsLetter(char c);
		bool IsLetterOrNumber(char c);

		void Number();
		void Identifier();
		void String();

		uint64_t mStartPos;
		uint64_t mCurPos;
		uint64_t mLine;
		std::string mSource;
		std::vector<Token> mTokens;
	};
}