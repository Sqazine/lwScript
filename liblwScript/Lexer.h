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
	class LWSCRIPT_API Lexer
	{
	public:
		Lexer();
		~Lexer();

		const std::vector<Token*> &ScanTokens(std::wstring_view src);

	private:
		void ResetStatus();

		void ScanToken();

		bool IsMatchCurChar(wchar_t c);
		bool IsMatchCurCharAndStepOnce(wchar_t c);
		bool IsMatchNextChar(wchar_t c);
		bool IsMatchNextCharAndStepOnce(wchar_t c);

		wchar_t GetNextCharAndStepOnce();
		wchar_t GetNextChar();
		wchar_t GetCurCharAndStepOnce();
		wchar_t GetCurChar();

		void AddToken(TokenType type);
		void AddToken(TokenType type, std::wstring_view literal);

		bool IsAtEnd();

		bool IsNumber(wchar_t c);
		bool IsLetter(wchar_t c, bool isAscii);
		bool IsLetterOrNumber(wchar_t c, bool isAscii);

		void Number();
		void Identifier();
		void String();

		uint64_t mStartPos;
		uint64_t mCurPos;
		uint64_t mLine;
		std::wstring mSource;
		std::vector<Token*> mTokens;
	};
}