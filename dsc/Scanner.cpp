#include <cassert>
#include "Scanner.h"

namespace dsc
{
	//--------------------------------------------------------------------------------
	Scanner::Scanner(const char* source)
	{
		assert(source);

		m_source = source;
		m_curString[0] = '\0';
		m_curStringIdx = 0;
		m_curIdx = 0;
		m_curLine = 1;
		m_curCol = 1;
		if (source)
			m_sourceLen = (uint32) strlen(source);
		else
			m_sourceLen = 0;
		m_prevTokenType = TOKEN_ERROR;
	}

	//--------------------------------------------------------------------------------
	static inline bool IsLetter(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z' || (c == '_'));
	}

	//--------------------------------------------------------------------------------
	static inline bool IsDigit(char c)
	{
		return (c >= '0' && c <= '9');
	}

	//--------------------------------------------------------------------------------
	static inline bool IsWhitespace(char c)
	{
		return (c == '\r' || c == '\n' || c == ' ' || c == '\t');
	}

	//--------------------------------------------------------------------------------
	static inline bool IsSymbol(char c)
	{
		switch (c)
		{
		case '/':
		case '*':
		case '-':
		case '+':
		case '%':
		case '=':
		case '<':
		case '>':
		case '.':
		case ':':
		case '(':
		case ')':
		case '{':
		case '}':
		case '&':
		case '|':
		case '!':
		case ',':
		case ';':
			return true;

		default:
			return false;
		}
	}

	//--------------------------------------------------------------------------------
	static inline uint32 GetTokenType(const char* str)
	{
		//figure out whether given string is an identifier or a keyword

		if (strcmp(str, "class") == 0)
			return TOKEN_CLASS;
		else if (strcmp(str, "super") == 0)
			return TOKEN_SUPER;
		else if (strcmp(str, "true") == 0)
			return TOKEN_TRUE;
		else if (strcmp(str, "false") == 0)
			return TOKEN_FALSE;
		else if (strcmp(str, "native") == 0)
			return TOKEN_NATIVE;
		else if (strcmp(str, "while") == 0)
			return TOKEN_WHILE;
		else if (strcmp(str, "if") == 0)
			return TOKEN_IF;
		else if (strcmp(str, "else") == 0)
			return TOKEN_ELSE;
		else if (strcmp(str, "return") == 0)
			return TOKEN_RETURN;
		else if (strcmp(str, "import") == 0)
			return TOKEN_IMPORT;
		else if (strcmp(str, "new") == 0)
			return TOKEN_NEW;
		else if (strcmp(str, "extends") == 0)
			return TOKEN_EXTENDS;
		else if (strcmp(str, "null") == 0)
			return TOKEN_NULL;
		else
			return TOKEN_IDENTIFIER;
	}

	//--------------------------------------------------------------------------------
	void Scanner::TakeIt()
	{
		if (CurChar() == '\0')
			return;

		if (CurChar() == '\n')
		{
			++m_curLine;
			m_curCol = 1;
		}
		else if (CurChar() != '\r')
		{
			++m_curCol;
		}

		m_curString[m_curStringIdx] = CurChar();
		++m_curStringIdx;
		++m_curIdx;

		if (m_curIdx > m_sourceLen)
			m_curIdx = m_sourceLen;
	}

	//--------------------------------------------------------------------------------
	void Scanner::IgnoreIt()
	{
		if (CurChar() == '\0')
			return;

		if (CurChar() == '\n')
		{
			++m_curLine;
			m_curCol = 1;
		}
		else if (CurChar() != '\r')
		{
			++m_curCol;
		}

		++m_curIdx;

		if (m_curIdx > m_sourceLen)
			m_curIdx = m_sourceLen;
	}

	//--------------------------------------------------------------------------------
	char Scanner::CurChar() const
	{
		if (m_curIdx < m_sourceLen)
			return m_source[m_curIdx];
		else
			return '\0';
	}

	//--------------------------------------------------------------------------------
	char Scanner::NextChar() const
	{
		if ((m_curIdx + 1) < m_sourceLen)
			return m_source[m_curIdx + 1];
		else
			return '\0';
	}

	//--------------------------------------------------------------------------------
	uint32 Scanner::ScanToken()
	{
		ClearCurString();

		//if source is empty, return EOF
		if (m_sourceLen == 0)
			return TOKEN_EOF;

		//scan whitespace first
		ScanWhitespace();

		if (IsLetter(CurChar()))
		{
			ScanIdentifier();

			//check whether identifier or a keyword
			m_curString[m_curStringIdx] = '\0';
			return GetTokenType(m_curString);
		}
		else if (CurChar() == '\"')
		{
			return ScanStringLiteral();
		}
		else if (IsSymbol(CurChar()))
		{
			//line comment //
			if (CurChar() == '/' && NextChar() == '/')
				return ScanLineComment();
			//bracketed comment /* */
			else if (CurChar() == '/' && NextChar() == '*')
				return ScanBracketedComment();
			//number literal .45764576
			else if (CurChar() == '.' && IsDigit(NextChar()))
				return ScanNumberLiteral();
			//symbol
			else
				return ScanSymbol();
		}
		else if (IsDigit(CurChar()))
		{
			return ScanNumberLiteral();
		}
		else if (CurChar() == 0)
		{
			return TOKEN_EOF;
		}
		else
		{
			TakeIt();
			return TOKEN_ERROR;
		}
	}

	//--------------------------------------------------------------------------------
	void Scanner::ScanWhitespace()
	{
		while (IsWhitespace(CurChar()))
			IgnoreIt();
	}

	//--------------------------------------------------------------------------------
	uint32 Scanner::ScanStringLiteral()
	{
		IgnoreIt();	//take opening "
		while (CurChar() != '\0' && CurChar() != '\"')
			TakeIt();

		if (CurChar() == '\0')
			return TOKEN_ERROR;

		IgnoreIt();	//take closing "

		return TOKEN_STRING_LITERAL;
	}

	//--------------------------------------------------------------------------------
	uint32 Scanner::ScanIdentifier()
	{
		TakeIt();	//take first letter
		while (IsLetter(CurChar()) || IsDigit(CurChar()))
			TakeIt();

		if (CurChar() == '\0')
			return TOKEN_ERROR;

		return TOKEN_IDENTIFIER;
	}

	//--------------------------------------------------------------------------------
	uint32 Scanner::ScanNumberLiteral()
	{
		while (IsDigit(CurChar()))
			TakeIt();

		if (CurChar() != '.')
			return TOKEN_INTEGER_LITERAL;

		if (m_curStringIdx == 0)
		{
			InsertChar('0');
		}

		TakeIt();	//take '.'

		if (!IsDigit(CurChar()))
			return TOKEN_ERROR;

		while (IsDigit(CurChar()))
			TakeIt();

		if (CurChar() == 'e' || CurChar() == 'E')
		{
			TakeIt();
			if (CurChar() == '-' || CurChar() == '+')
				TakeIt();	//take sign

			if (!IsDigit(CurChar()))
				return TOKEN_ERROR;

			while (IsDigit(CurChar()))
				TakeIt();
		}

		return TOKEN_FLOAT_LITERAL;
	}

	//--------------------------------------------------------------------------------
	uint32 Scanner::ScanSymbol()
	{
		switch (CurChar())
		{
		case '/':
			TakeIt();
			return TOKEN_DIVIDE;

		case '*':
			TakeIt();
			return TOKEN_MULTIPLY;

		case '-':
			TakeIt();
			if (m_prevTokenType == TOKEN_ASSIGN
				|| m_prevTokenType == TOKEN_NOT_EQUALS
				|| m_prevTokenType == TOKEN_EQUALS
				|| m_prevTokenType == TOKEN_DIVIDE
				|| m_prevTokenType == TOKEN_MULTIPLY
				|| m_prevTokenType == TOKEN_MODULO
				|| m_prevTokenType == TOKEN_LT
				|| m_prevTokenType == TOKEN_LTEQ
				|| m_prevTokenType == TOKEN_GT
				|| m_prevTokenType == TOKEN_GTEQ
				|| m_prevTokenType == TOKEN_OPEN_BRACKET
				|| m_prevTokenType == TOKEN_OR
				|| m_prevTokenType == TOKEN_AND
				|| m_prevTokenType == TOKEN_NOT
				|| m_prevTokenType == TOKEN_COMMA)
			{
				return TOKEN_UNARY_MINUS;
			}
			else
			{
				return TOKEN_MINUS;
			}

		case '+':
			TakeIt();
			return TOKEN_PLUS;

		case '%':
			TakeIt();
			return TOKEN_MODULO;

		case '=':
			TakeIt();
			if (CurChar() == '=')
			{
				TakeIt();
				return TOKEN_EQUALS;
			}
			return TOKEN_ASSIGN;

		case '<':
			TakeIt();
			if (CurChar() == '=')
			{
				TakeIt();
				return TOKEN_LTEQ;
			}
			return TOKEN_LT;

		case '>':
			TakeIt();
			if (CurChar() == '=')
			{
				TakeIt();
				return TOKEN_GTEQ;
			}
			return TOKEN_GT;

		case '.':
			TakeIt();
			return TOKEN_DOT;

		case '(':
			TakeIt();
			return TOKEN_OPEN_BRACKET;

		case ')':
			TakeIt();
			return TOKEN_CLOSE_BRACKET;

		case '{':
			TakeIt();
			return TOKEN_OPEN_CURLY_BRACKET;

		case '}':
			TakeIt();
			return TOKEN_CLOSE_CURLY_BRACKET;

		case '&':
			TakeIt();
			if (CurChar() == '&')
			{
				TakeIt();
				return TOKEN_AND;
			}
			else
				return TOKEN_ERROR;

		case '|':
			TakeIt();
			if (CurChar() == '|')
			{
				TakeIt();
				return TOKEN_OR;
			}
			else
				return TOKEN_ERROR;

		case '!':
			TakeIt();
			if (CurChar() == '=')
			{
				TakeIt();
				return TOKEN_NOT_EQUALS;
			}
			else
				return TOKEN_NOT;

		case ',':
			TakeIt();
			return TOKEN_COMMA;

		case ';':
			TakeIt();
			return TOKEN_SEMICOLON;

		default:
			return TOKEN_ERROR;
		}
	}

	//--------------------------------------------------------------------------------
	uint32 Scanner::ScanBracketedComment()
	{
		IgnoreIt();	//take /
		IgnoreIt();	//take *

		while (CurChar() != '\0' && !(CurChar() == '*' && NextChar() == '/'))
		{
			TakeIt();
		}

		if (CurChar() == '\0' || NextChar() == '\0')
			return TOKEN_ERROR;

		IgnoreIt();	//take *
		IgnoreIt();	//take /

		return TOKEN_BRACKETED_COMMENT;
	}

	//--------------------------------------------------------------------------------
	uint32 Scanner::ScanLineComment()
	{
		IgnoreIt();	//take first /
		IgnoreIt();	//take second /

		while (CurChar() != '\n' && CurChar() != '\r' && CurChar() != '\0')
			TakeIt();

		return TOKEN_LINE_COMMENT;
	}

	//--------------------------------------------------------------------------------
	void Scanner::ClearCurString()
	{
		m_curStringIdx = 0;
	}

	//--------------------------------------------------------------------------------
	void Scanner::InsertChar(char c)
	{
		assert((m_curStringIdx + 1) < 1024);

		m_curString[m_curStringIdx] = c;
		++m_curStringIdx;
	}

	//--------------------------------------------------------------------------------
	Token Scanner::Scan()
	{
		uint32 tokenType = ScanToken();

		if (tokenType != TOKEN_BRACKETED_COMMENT && tokenType != TOKEN_LINE_COMMENT)
			m_prevTokenType = tokenType;

		m_curString[m_curStringIdx] = '\0';

		return Token(tokenType, m_curString, m_curLine, m_curCol);
	}

	//--------------------------------------------------------------------------------
	bool Scanner::Done()
	{
		return (m_prevTokenType == TOKEN_EOF);
	}

	//--------------------------------------------------------------------------------
}