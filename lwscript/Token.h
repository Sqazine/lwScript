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
	DOT,				   // .
	COMMA,				   // ,
	COLON,				   // :
	SEMICOLON,			   // ;
	LBRACKET,			   // [
	RBRACKET,			   // ]
	LBRACE,				   // {
	RBRACE,				   // }
	LPAREN,				   // (
	RPAREN,				   // )
	PLUS,				   // +
	MINUS,				   // -
	ASTERISK,			   // *
	SLASH,				   // /
	MOD,				   // %
	EQUAL,				   // =
	LESS,				   // <
	GREATER,			   // >
	BIT_AND,			   // &
	BIT_OR,				   // |
	BIT_XOR,			   // ^
	BIT_NOT,			   // ~
	QUESTION,			   // ?
	BANG,				   // !
	BIT_LEFT_SHIFT,		   // <<
	BIT_RIGHT_SHIFT,	   // >>
	PLUS_EQUAL,			   // +=
	MINUS_EQUAL,		   // -=
	ASTERISK_EQUAL,		   // *=
	SLASH_EQUAL,		   // /=
	MOD_EQUAL,			   // %=
	BIT_AND_EQUAL,		   // &=
	BIT_OR_EQUAL,		   // |=
	BIT_XOR_EQUAL,		   // ^=
	BIT_LEFT_SHIFT_EQUAL,  // <<=
	BIT_RIGHT_SHIFT_EQUAL, // >>
	EEQUAL,				   // ==
	LEQUAL,				   // <=
	GEQUAL,				   // >=
	BEQUAL,				   // !=
	AND,				   // &&
	OR,					   // ||
	LET,				   // let
	IF,					   // if
	ELSE,				   // else
	TRUE,				   // true
	FALSE,				   // false
	NIL,				   // nil
	WHILE,				   // while
	FUNCTION,			   // function
	STRUCT,				   // struct
	RETURN,				   // return
	REF,				   // ref

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