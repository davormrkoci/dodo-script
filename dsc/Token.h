#if !defined(DSC_TOKEN_H_)
#define DSC_TOKEN_H_

#include <string>
#include "BaseTypes.h"

namespace dsc
{
	enum
	{
		//keywords
		TOKEN_CLASS = 0,
		TOKEN_SUPER,
		TOKEN_TRUE,
		TOKEN_FALSE,
		TOKEN_NATIVE,
		TOKEN_WHILE,
		TOKEN_IF,
		TOKEN_ELSE,
		TOKEN_RETURN,
		TOKEN_IMPORT,
		TOKEN_IDENTIFIER,
		TOKEN_EOF,
		TOKEN_INTEGER_LITERAL,
		TOKEN_FLOAT_LITERAL,
		TOKEN_STRING_LITERAL,
		TOKEN_DIVIDE,
		TOKEN_MULTIPLY,
		TOKEN_UNARY_MINUS,
		TOKEN_MINUS,
		TOKEN_PLUS,
		TOKEN_MODULO,
		TOKEN_EQUALS,
		TOKEN_NOT_EQUALS,
		TOKEN_LTEQ,
		TOKEN_LT,
		TOKEN_GTEQ,
		TOKEN_GT,
		TOKEN_DOT,
		TOKEN_EXTENDS,
		TOKEN_OPEN_BRACKET,
		TOKEN_CLOSE_BRACKET,
		TOKEN_OPEN_CURLY_BRACKET,
		TOKEN_CLOSE_CURLY_BRACKET,
		TOKEN_AND,
		TOKEN_OR,
		TOKEN_NOT,
		TOKEN_ASSIGN,
		TOKEN_COMMA,
		TOKEN_SEMICOLON,
		TOKEN_NEW,
		TOKEN_NULL,

		//comments
		TOKEN_BRACKETED_COMMENT,
		TOKEN_LINE_COMMENT,

		//error
		TOKEN_ERROR,
	};

	class Token
	{
	public:
		Token();
		Token(uint32 type, const char* spelling, uint32 line, uint32 col);
		uint32 GetType() const;
		const char* GetSpelling() const;
		uint32 GetLine() const;
		uint32 GetCol() const;

	private:
		uint32 m_type;
		std::string m_spelling;
		uint32 m_line;
		uint32 m_col;
	};

	const std::string TokenTypeToString(uint32 type);
	const std::string ToString(const Token& tok);
}

#endif