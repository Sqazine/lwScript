#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>
namespace lwScript
{

	enum class TokenType
	{
		NUMBER = 0,
		STRING,
		IDENTIFIER,

		DOT,	   // .
		COMMA,	   // ,
		COLON,	   // :
		SEMICOLON, // ;
		LBRACKET,  // [
		RBRACKET,  // ]
		LBRACE,	   // {
		RBRACE,	   // }
		LPAREN,	   // (
		RPAREN,	   // )
		PLUS,	   // +
		MINUS,	   // -
		ASTERISK,  // *
		SLASH,	   // /
		EQUAL,	   // =
		LESS,	   // <
		GREATER,   // >
		SHARP,	   // #
		EEQUAL,	   // ==
		LEQUAL,	   // <=
		GEQUAL,	   // >=
		BEQUAL,	   // !=
		AND,	   // &&
		OR,		   // ||
		LET,	   // var
		IF,		   // if
		ELSE,	   // else
		TRUE,	   // true
		FALSE,	   // false
		NIL,	   // nil
		WHILE,	   // while
		FUNCTION,  // fn
		STRUCT,	   // struct
		RETURN,	   // return
		REF,	   // ref

		UNKNOWN,
		END
	};

	struct Token
	{
		Token(TokenType type, std::string_view literal, uint64_t line) : type(type), literal(literal), line(line) {}

		TokenType type;
		std::string literal;
		uint64_t line;
	};

	inline std::ostream &operator<<(std::ostream &stream, const Token &token)
	{
		return stream << token.literal << "," << token.line;
	}
}