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
		TOKEN_ELLIPSIS,				 // ...
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
		TOKEN_LPAREN_LBRACE,		 // ({
		TOKEN_RBRACE_RPAREN,		 // })
		TOKEN_LESS_LESS,			 // <<
		TOKEN_GREATER_GREATER,		 // >>
		TOKEN_PLUS_PLUS,			 // ++
		TOKEN_MINUS_MINUS,			 // --
		TOKEN_PLUS_EQUAL,			 // +=
		TOKEN_MINUS_EQUAL,			 // -=
		TOKEN_ASTERISK_EQUAL,		 // *=
		TOKEN_SLASH_EQUAL,			 // /=
		TOKEN_PERCENT_EQUAL,		 // %=
		TOKEN_AMPERSAND_EQUAL,		 // &=
		TOKEN_VBAR_EQUAL,			 // |=
		TOKEN_CARET_EQUAL,			 // ^=
		TOKEN_LESS_LESS_EQUAL,		 // <<=
		TOKEN_GREATER_GREATER_EQUAL, // >>=
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
		TOKEN_CLASS,				 // class
		TOKEN_THIS,					 // this
		TOKEN_BASE,					 // base
		TOKEN_RETURN,				 // return
		TOKEN_STATIC,				 // static
		TOKEN_CONST,				 // const
		TOKEN_BREAK,				 // break
		TOKEN_CONTINUE,				 // continue
		TOKEN_IMPORT,				 // import
		TOKEN_MODULE,				 // module
		TOKEN_SWITCH,				 // switch
		TOKEN_DEFAULT,				 // default
		TOKEN_MATCH,				 // match
		TOKEN_ENUM,					 // enum
		TOKEN_U8,					 // u8
		TOKEN_U16,					 // u16
		TOKEN_U32,					 // u32
		TOKEN_U64,					 // u64
		TOKEN_I8,					 // i8
		TOKEN_I16,					 // i16
		TOKEN_I32,					 // i32
		TOKEN_I64,					 // i64
		TOKEN_F32,					 // f32
		TOKEN_F64,					 // f64
		TOKEN_BOOL,					 // bool
		TOKEN_CHAR,					 // char
		TOKEN_ANY,					 // any
		TOKEN_VOID,					 // void
		TOKEN_AS,					 // as
		TOKEN_NEW,					 // new
		TOKEN_EOF,
	};

	struct Token
	{
		Token() : type(TOKEN_EOF), literal(L""), line(-1), column(-1), pos(-1) {}
		Token(TokenType type, std::wstring_view literal, uint64_t line, uint64_t column, uint64_t pos) : type(type), literal(literal), line(line), column(column), pos(pos) {}

		std::wstring ToString() const
		{
			return L"\"" + literal + L"\"(" + std::to_wstring(line) + L"," + std::to_wstring(column) + L")";
		}

		TokenType type;
		std::wstring literal;
		uint64_t line;
		uint64_t column;
		uint64_t pos;
	};

	inline std::wostream &operator<<(std::wostream &stream, const Token &token)
	{
		return stream << token.ToString();
	}
}