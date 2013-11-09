#if !defined(DSC_PARSER_H_)
#define DSC_PARSER_H_

#include "ScriptSource.h"
#include "Token.h"

namespace dsc
{
	class Parser
	{
		DSC_NOCOPY(Parser)

		typedef std::list<Token> TokenList;
	public:
		Parser();
		ScriptSourceCPtr ParseScript(const char* source);
		void GetCurrentLocation(uint32& line, uint32& col) const;

	private:
		DataSrcCPtr ParseClassData();
		DataSrcCPtr ParseLocalData();
		FunctionSrcCPtr ParseFunction();
		StatementSrcCPtr ParseStatement();
		StatementSrcCPtr ParseStatementBlock();
		ExpressionSrcCPtr ParseExpression();
		FunctionCallSrcCPtr ParseFunctionCall();
		void ParseScriptClassName(std::string& name);

		int32 GetOperatorPriority(uint32 tokenType) const;
		bool IsFunctionCall() const;
		bool IsLocalData() const;
		bool IsOperator(uint32 tokenType) const;
		bool IsFunctionParameter(uint32 tokenType) const;
		uint32 GetDistTo(uint32 tokenType) const;
		void ResetTokenItt();
		void Accept(uint32 tokenType);
		Token PrevToken() const;
		Token CurToken() const;
		Token NextToken() const;
		Token NextNextToken() const;
		Token NextNextNextToken() const;

	private:
		Token m_prevToken;
		TokenList m_tokens;
		TokenList::const_iterator m_curTokenItt;

		ScriptSourceCPtr m_cpScriptSource;
		std::vector<std::string> m_comments;
	};
}

#endif