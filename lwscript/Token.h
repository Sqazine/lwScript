#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>

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
	EEQUAL,	   // ==
	LEQUAL,	   // <=
	GEQUAL,	   // >=
	BEQUAL,	   // !=
	AND,	   // &&
	OR,		   // ||
	LET,	   // let
	IF,		   // if
	ELSE,	   // else
	TRUE,	   // true
	FALSE,	   // false
	NIL,	   // nil
	WHILE,	   // while
	FUNCTION,  // function
	STRUCT,	   // struct
	RETURN,	   // return

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