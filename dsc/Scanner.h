#if !defined(DSC_SCANNER_H_)
#define DSC_SCANNER_H_

#include "Token.h"
#include "ClassUtils.h"

namespace dsc
{
	class Scanner
	{
		DSC_NOCOPY(Scanner)
	public:
		Scanner(const char* source);
		Token Scan();
		bool Done();

	private:
		void TakeIt();
		void IgnoreIt();

		char CurChar() const;
		char NextChar() const;

		void ClearCurString();
		void InsertChar(char c);

		//Scan helpers
		void ScanWhitespace();
		uint32 ScanToken();
		uint32 ScanIdentifier();
		uint32 ScanNumberLiteral();
		uint32 ScanOperator();
		uint32 ScanBracketedComment();
		uint32 ScanLineComment();
		uint32 ScanSymbol();
		uint32 ScanStringLiteral();

		Scanner();	//not implemented

	private:
		const char* m_source;
		char m_curString[1024];
		uint32 m_curStringIdx;
		uint32 m_curIdx;
		uint32 m_curLine;
		uint32 m_curCol;
		uint32 m_sourceLen;
		uint32 m_prevTokenType;
	};
}

#endif