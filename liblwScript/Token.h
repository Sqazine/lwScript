#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>
namespace lws
{
	enum TokenType
	{
		TOKEN_NUMBER = 0,
		TOKEN_STRING,
		TOKEN_IDENTIFIER,
		TOKEN_DOT,					 // .
		TOKEN_COMMA,				 // ,
		TOKEN_COLON,				 // :
		TOKEN_SEMICOLON,			 // ;
		TOKEN_LBRACKET,				 // [
		TOKEN_RBRACKET,				 // ]
		TOKEN_LBRACE,				 // {
		TOKEN_RBRACE,				 // }
		TOKEN_LPAREN,				 // (
		TOKEN_RPAREN,				 // )
		TOKEN_PLUS,					 // +
		TOKEN_MINUS,				 // -
		TOKEN_ASTERISK,				 // *
		TOKEN_SLASH,				 // /
		TOKEN_PERCENT,				 // %
		TOKEN_EQUAL,				 // =
		TOKEN_LESS,					 // <
		TOKEN_GREATER,				 // >
		TOKEN_AMPERSAND,			 // &
		TOKEN_VBAR,					 // |
		TOKEN_CARET,				 // ^
		TOKEN_TILDE,				 // ~
		TOKEN_QUESTION,				 // ?
		TOKEN_BANG,					 // !
		TOKEN_LESS_LESS,			 // <<
		TOKEN_GREATER_GREATER,		 // >>
		TOKEN_PLUS_EQUAL,			 // +=
		TOKEN_MINUS_EQUAL,			 // -=
		TOKEN_ASTERISK_EQUAL,		 // *=
		TOKEN_SLASH_EQUAL,			 // /=
		TOKEN_PERCENT_EQUAL,		 // %=
		TOKEN_AMPERSAND_EQUAL,		 // &=
		TOKEN_VBAR_EQUAL,			 // |=
		TOKEN_CARET_EQUAL,			 // ^=
		TOKEN_LESS_LESS_EQUAL,		 // <<=
		TOKEN_GREATER_GREATER_EQUAL, // >>
		TOKEN_EQUAL_EQUAL,			 // ==
		TOKEN_LESS_EQUAL,			 // <=
		TOKEN_GREATER_EQUAL,		 // >=
		TOKEN_BANG_EQUAL,			 // !=
		TOKEN_AMPERSAND_AMPERSAND,	 // &&
		TOKEN_VBAR_VBAR,			 // ||
		TOKEN_LET,					 // let
		TOKEN_IF,					 // if
		TOKEN_ELSE,					 // else
		TOKEN_TRUE,					 // true
		TOKEN_FALSE,				 // false
		TOKEN_NULL,					 // null
		TOKEN_WHILE,				 // while
		TOKEN_FOR,					 // for
		TOKEN_FUNCTION,				 // fn
		TOKEN_FIELD,				 // field
		TOKEN_RETURN,				 // return
		TOKEN_STATIC,				 // static
		TOKEN_CONST,				 // const
		TOKEN_BREAK,				 // break
		TOKEN_CONTINUE,				 // continue
		TOKEN_IMPORT,				 // import
		TOKEN_EXPORT,				 // export
		TOKEN_MODULE,				 // module
		TOKEN_TRY,					 // try
		TOKEN_CATCH,				 // catch
		TOKEN_SWITCH,				 // switch
		TOKEN_CASE,					 // case
		TOKEN_END
	};

	struct Token
	{
		Token(TokenType type, std::wstring_view literal, uint64_t line, uint64_t column) : type(type), literal(literal), line(line), column(column) {}

		TokenType type;
		std::wstring literal;
		uint64_t line;
		uint64_t column;
	};

	inline std::wostream &operator<<(std::wostream &stream, const Token &token)
	{
		return stream << token.literal << L"(" << token.line << "," << token.column << L")";
	}
}