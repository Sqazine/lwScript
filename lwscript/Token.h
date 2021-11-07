#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>
namespace lws
{
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
		PERCENT,			   // %
		EQUAL,				   // =
		LESS,				   // <
		GREATER,			   // >
		AMPERSAND,			   // &
		VBAR,				   // |
		CARET,				   // ^
		TILDE,				   // ~
		QUESTION,			   // ?
		BANG,				   // !
		LESS_LESS,			   // <<
		GREATER_GREATER,	   // >>
		PLUS_EQUAL,			   // +=
		MINUS_EQUAL,		   // -=
		ASTERISK_EQUAL,		   // *=
		SLASH_EQUAL,		   // /=
		PERCENT_EQUAL,		   // %=
		AMPERSAND_EQUAL,	   // &=
		VBAR_EQUAL,			   // |=
		CARET_EQUAL,		   // ^=
		LESS_LESS_EQUAL,	   // <<=
		GREATER_GREATER_EQUAL, // >>
		EQUAL_EQUAL,		   // ==
		LESS_EQUAL,			   // <=
		GREATER_EQUAL,		   // >=
		BANG_EQUAL,			   // !=
		AMPERSAND_AMPERSAND,   // &&
		VBAR_VBAR,			   // ||
		LET,				   // let
		IF,					   // if
		ELSE,				   // else
		TRUE,				   // true
		FALSE,				   // false
		NIL,				   // nil
		WHILE,				   // while
		FOR,				   // for
		FUNCTION,			   // function
		LAMBDA,				   // lambda
		CLASS,				   // class
		RETURN,				   // return
		NEW,				   // new

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

	inline std::ostream& operator<<(std::ostream& stream, const Token& token)
	{
		return stream << token.literal << "," << token.line;
	}
}