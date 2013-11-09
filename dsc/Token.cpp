#include <cassert>
#include "Token.h"

namespace dsc
{
	Token::Token()
	: m_type(TOKEN_ERROR), m_spelling(""), m_line(0), m_col(0)
	{
	}

	Token::Token(uint32 type, const char* spelling, uint32 line, uint32 col)
	: m_type(type), m_spelling(spelling), m_line(line), m_col(col)
	{
	}

	uint32 Token::GetType() const
	{
		return m_type;
	}

	uint32 Token::GetLine() const
	{
		return m_line;
	}

	uint32 Token::GetCol() const
	{
		return m_col;
	}

	const char* Token::GetSpelling() const
	{
		return m_spelling.c_str();
	}

	const std::string TokenTypeToString(uint32 type)
	{
		switch (type)
		{
		case TOKEN_CLASS:
			return "class";
		case TOKEN_SUPER:
			return "super";
		case TOKEN_TRUE:
			return "true";
		case TOKEN_FALSE:
			return "false";
		case TOKEN_NATIVE:
			return "native";
		case TOKEN_WHILE:
			return "while";
		case TOKEN_IF:
			return "if";
		case TOKEN_ELSE:
			return "else";
		case TOKEN_RETURN:
			return "return";
		case TOKEN_IDENTIFIER:
			return "identifier";
		case TOKEN_EOF:
			return "eof";
		case TOKEN_INTEGER_LITERAL:
			return "integer literal";
		case TOKEN_FLOAT_LITERAL:
			return "float literal";
		case TOKEN_STRING_LITERAL:
			return "string literal";
		case TOKEN_DIVIDE:
			return "/";
		case TOKEN_MULTIPLY:
			return "*";
		case TOKEN_UNARY_MINUS:
			return "unary minus";
		case TOKEN_MINUS:
			return "-";
		case TOKEN_PLUS:
			return "+";
		case TOKEN_MODULO:
			return "%";
		case TOKEN_EQUALS:
			return "==";
		case TOKEN_NOT_EQUALS:
			return "!=";
		case TOKEN_LTEQ:
			return "<=";
		case TOKEN_LT:
			return "<";
		case TOKEN_GTEQ:
			return ">=";
		case TOKEN_GT:
			return ">";
		case TOKEN_DOT:
			return ".";
		case TOKEN_EXTENDS:
			return "extends";
		case TOKEN_OPEN_BRACKET:
			return "(";
		case TOKEN_CLOSE_BRACKET:
			return ")";
		case TOKEN_OPEN_CURLY_BRACKET:
			return "{";
		case TOKEN_CLOSE_CURLY_BRACKET:
			return "}";
		case TOKEN_AND:
			return "&&";
		case TOKEN_OR:
			return "||";
		case TOKEN_NOT:
			return "!";
		case TOKEN_ASSIGN:
			return "=";
		case TOKEN_BRACKETED_COMMENT:
			return "/* */";
		case TOKEN_LINE_COMMENT:
			return "//";
		case TOKEN_ERROR:
			return "error";
		case TOKEN_COMMA:
			return ",";
		case TOKEN_SEMICOLON:
			return ";";
		case TOKEN_NEW:
			return "new";
		case TOKEN_NULL:
			return "null";
		default:
			assert(false);
			return "";
		}
	}

	const std::string ToString(const Token& tok)
	{
		std::string res = "Type: ";
		res += TokenTypeToString(tok.GetType());
		res += ", Spelling: ";
		res += tok.GetSpelling();

		return res;
	}
}