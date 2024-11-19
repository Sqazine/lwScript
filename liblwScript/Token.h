#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>
#include "Config.h"
namespace lwscript
{
	enum class TokenKind
	{
		NUMBER = 0,			   // number literal,like 100
		STRING,				   // string literal,like "AABB"
		CHARACTER,			   // character literal,like 'A'
		IDENTIFIER,			   // variable like A
		DOT,				   // .
		ELLIPSIS,			   // ...
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
		LPAREN_LBRACE,		   // ({
		RBRACE_RPAREN,		   // })
		LESS_LESS,			   // <<
		GREATER_GREATER,	   // >>
		PLUS_PLUS,			   // ++
		MINUS_MINUS,		   // --
		PLUS_EQUAL,			   // +=
		MINUS_EQUAL,		   // -=
		ASTERISK_EQUAL,		   // *=
		SLASH_EQUAL,		   // /=
		PERCENT_EQUAL,		   // %=
		AMPERSAND_EQUAL,	   // &=
		VBAR_EQUAL,			   // |=
		CARET_EQUAL,		   // ^=
		LESS_LESS_EQUAL,	   // <<=
		GREATER_GREATER_EQUAL, // >>=
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
		NIL,				   // null
		WHILE,				   // while
		FOR,				   // for
		FUNCTION,			   // fn
		CLASS,				   // class
		STRUCT,				   // struct
		THIS,				   // this
		BASE,				   // base
		RETURN,				   // return
		STATIC,				   // static
		CONST,				   // const
		BREAK,				   // break
		CONTINUE,			   // continue
		IMPORT,				   // import
		MODULE,				   // module
		SWITCH,				   // switch
		DEFAULT,			   // default
		MATCH,				   // match
		ENUM,				   // enum
		U8,					   // u8
		U16,				   // u16
		U32,				   // u32
		U64,				   // u64
		I8,					   // i8
		I16,				   // i16
		I32,				   // i32
		I64,				   // i64
		F32,				   // f32
		F64,				   // f64
		BOOL,				   // bool
		CHAR,				   // char
		ANY,				   // any
		VOID,				   // void
		AS,					   // as
		NEW,				   // new
		END,
	};

	struct LWSCRIPT_API Token
	{
		Token() : kind(TokenKind::END), literal(TEXT("")), line(-1), column(-1), pos(-1) {}
		Token(TokenKind kind, STD_STRING_VIEW literal, uint64_t line, uint64_t column, uint64_t pos) : kind(kind), literal(literal), line(line), column(column), pos(pos) {}

		STD_STRING ToString() const
		{
			return TEXT("\"") + literal + TEXT("\"(") + TO_STRING(line) + TEXT(",") + TO_STRING(column) + TEXT(")");
		}

		TokenKind kind;
		STD_STRING literal;
		uint64_t line;
		uint64_t column;
		uint64_t pos;
	};

	inline STD_OSTREAM &operator<<(STD_OSTREAM &stream, const Token &token)
	{
		return stream << token.ToString();
	}
}