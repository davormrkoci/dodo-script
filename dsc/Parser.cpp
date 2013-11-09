#include <stdarg.h>
#include "Parser.h"
#include "Scanner.h"
#include "Compiler.h"

namespace dsc
{
	static const char* FORMAT(const char *fmt, ...)
	{
		static char str[1024];

		va_list ap;
		va_start(ap, fmt);
		_vsnprintf(str, 1024, fmt, ap);
		va_end(ap);

		return str;
	}

	//--------------------------------------------------------------------------------
	Parser::Parser()
	{
	}

	//--------------------------------------------------------------------------------
	void Parser::Accept(uint32 tokenType)
	{
		if (m_curTokenItt != m_tokens.end())
			m_prevToken = *m_curTokenItt;

		if (CurToken().GetType() == tokenType)
		{
			++m_curTokenItt;

			//skip comments
			while (m_curTokenItt != m_tokens.end()
				&& (CurToken().GetType() == TOKEN_LINE_COMMENT || CurToken().GetType() == TOKEN_BRACKETED_COMMENT))
			{
				m_comments.push_back(std::string(CurToken().GetSpelling()));
				++m_curTokenItt;
			}
			return;
		}
		else
		{
			throw CompilerException(FORMAT("Expected token of type \"%s\", but encountered \"%s\".  Class %s, line %u.",
					TokenTypeToString(tokenType).c_str(), CurToken().GetSpelling(), m_cpScriptSource->GetName(), CurToken().GetLine()));
		}
	}

	//--------------------------------------------------------------------------------
	Token Parser::PrevToken() const
	{
		return m_prevToken;
	}


	//--------------------------------------------------------------------------------
	Token Parser::CurToken() const
	{
		if (m_curTokenItt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);
		else
			return *m_curTokenItt;
	}

	//--------------------------------------------------------------------------------
	Token Parser::NextToken() const
	{
		if (m_curTokenItt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);

		TokenList::const_iterator itt = m_curTokenItt;
		++itt;

		//skip comments
		while (itt != m_tokens.end()
			&& ((*itt).GetType() == TOKEN_LINE_COMMENT || (*itt).GetType() == TOKEN_BRACKETED_COMMENT))
		{
			++itt;
		}

		if (itt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);
		else
			return *itt;
	}

	//--------------------------------------------------------------------------------
	Token Parser::NextNextToken() const
	{
		if (m_curTokenItt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);

		TokenList::const_iterator itt = m_curTokenItt;

		++itt;

		//skip comments
		while (itt != m_tokens.end()
			&& ((*itt).GetType() == TOKEN_LINE_COMMENT || (*itt).GetType() == TOKEN_BRACKETED_COMMENT))
		{
			++itt;
		}

		if (itt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);

		++itt;

		//skip comments
		while (itt != m_tokens.end()
			&& ((*itt).GetType() == TOKEN_LINE_COMMENT || (*itt).GetType() == TOKEN_BRACKETED_COMMENT))
		{
			++itt;
		}

		if (itt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);
		else
			return *itt;
	}

	//--------------------------------------------------------------------------------
	Token Parser::NextNextNextToken() const
	{
		if (m_curTokenItt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);

		TokenList::const_iterator itt = m_curTokenItt;

		++itt;

		//skip comments
		while (itt != m_tokens.end()
			&& ((*itt).GetType() == TOKEN_LINE_COMMENT || (*itt).GetType() == TOKEN_BRACKETED_COMMENT))
		{
			++itt;
		}

		if (itt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);

		++itt;

		//skip comments
		while (itt != m_tokens.end()
			&& ((*itt).GetType() == TOKEN_LINE_COMMENT || (*itt).GetType() == TOKEN_BRACKETED_COMMENT))
		{
			++itt;
		}

		if (itt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);

		++itt;

		//skip comments
		while (itt != m_tokens.end()
			&& ((*itt).GetType() == TOKEN_LINE_COMMENT || (*itt).GetType() == TOKEN_BRACKETED_COMMENT))
		{
			++itt;
		}

		if (itt == m_tokens.end())
			return Token(TOKEN_ERROR, "", 0, 0);
		else
			return *itt;
	}
	
	//--------------------------------------------------------------------------------
	void Parser::ResetTokenItt()
	{
		m_curTokenItt = m_tokens.begin();

		//skip comments
		while (m_curTokenItt != m_tokens.end()
			&& (CurToken().GetType() == TOKEN_LINE_COMMENT || CurToken().GetType() == TOKEN_BRACKETED_COMMENT))
		{
			m_comments.push_back(std::string(CurToken().GetSpelling()));
			++m_curTokenItt;
		}
	}

	//--------------------------------------------------------------------------------
	uint32 Parser::GetDistTo(uint32 tokenType) const
	{
		uint32 dist = 0;
		for (TokenList::const_iterator itt = m_curTokenItt; (itt != m_tokens.end() && (*itt).GetType() != tokenType); ++itt)
		{
			++dist;
		}

		return dist;
	}

	//--------------------------------------------------------------------------------
	void Parser::ParseScriptClassName(std::string& className)
	{
		Accept(TOKEN_IDENTIFIER);

		className = PrevToken().GetSpelling();
		while (CurToken().GetType() == TOKEN_DOT)
		{
			Accept(TOKEN_DOT);
			Accept(TOKEN_IDENTIFIER);

			className += '.';
			className += PrevToken().GetSpelling();
		}
	}

	//--------------------------------------------------------------------------------
	ScriptSourceCPtr Parser::ParseScript(const char* source)
	{
		m_tokens.clear();
		m_cpScriptSource.set(new ScriptSource());

		//scan source, and create a list of tokens
		Scanner scanner(source);
		while (!scanner.Done())
		{
			Token tok = scanner.Scan();
			m_tokens.push_back(tok);
			if (tok.GetType() == TOKEN_ERROR)
			{
				assert(false);
				throw CompilerException(FORMAT("Invalid token encountered \"%s\".  Class %s, line %u.",
					tok.GetSpelling(), m_cpScriptSource->GetName(), tok.GetLine()));
			}
		}
		ResetTokenItt();

		while (CurToken().GetType() == TOKEN_IMPORT)
		{
			m_comments.clear();

			Accept(TOKEN_IMPORT);

			std::string importClass;
			ParseScriptClassName(importClass);
			m_cpScriptSource->AddImportClass(importClass.c_str());

			Accept(TOKEN_SEMICOLON);
		}

		//add class comments
		for (uint32 i=0; i<m_comments.size(); ++i)
		{
			m_cpScriptSource->AddComment(m_comments[i].c_str());
		}

		//check if native script
		if (CurToken().GetType() == TOKEN_NATIVE)
		{
			Accept(TOKEN_NATIVE);
			m_cpScriptSource->SetNative(true);
		}

		//verify script keyword
		Accept(TOKEN_CLASS);

		//get script name
		std::string className;
		ParseScriptClassName(className);
		m_cpScriptSource->SetName(className.c_str());

		//get super script
		if (CurToken().GetType() == TOKEN_EXTENDS)
		{
			Accept(TOKEN_EXTENDS);

			//get parent script name
			std::string superClassName;
			ParseScriptClassName(superClassName);
			m_cpScriptSource->SetSuper(superClassName.c_str());
		}

		//opening '{'
		m_comments.clear();
		Accept(TOKEN_OPEN_CURLY_BRACKET);

		while (CurToken().GetType() != TOKEN_CLOSE_CURLY_BRACKET)
		{
			uint32 distSemicolon = GetDistTo(TOKEN_SEMICOLON);
			uint32 distOpenBracket = GetDistTo(TOKEN_OPEN_BRACKET);

			if (distSemicolon <= distOpenBracket)
			{
				//data
				DataSrcCPtr data = ParseClassData();
				m_cpScriptSource->AddDataMember(data);
			}
			else
			{
				//function
				FunctionSrcCPtr fnc = ParseFunction();

				if (fnc->IsConstructor())
					m_cpScriptSource->AddConstructor(fnc);
				else
					m_cpScriptSource->AddFunction(fnc);
			}
		}

		//closing '}'
		Accept(TOKEN_CLOSE_CURLY_BRACKET);
		Accept(TOKEN_SEMICOLON);
		
		return m_cpScriptSource;
	}

	//--------------------------------------------------------------------------------
	DataSrcCPtr Parser::ParseClassData()
	{
		DataSrcCPtr res = new DataSrc();
		res->SetLine(CurToken().GetLine());

		//add data comments
		for (uint32 i=0; i<m_comments.size(); ++i)
		{
			res->AddComment(m_comments[i].c_str());
		}

		//get data type
		std::string className;
		ParseScriptClassName(className);
		res->SetType(className.c_str());

		//get variable name
		Accept(TOKEN_IDENTIFIER);
		res->SetName(PrevToken().GetSpelling());

		//get ;
		m_comments.clear();
		Accept(TOKEN_SEMICOLON);

		return res;
	}

	//--------------------------------------------------------------------------------
	DataSrcCPtr Parser::ParseLocalData()
	{
		DataSrcCPtr res = new DataSrc();
		res->SetLine(CurToken().GetLine());

		//add data comments
		for (uint32 i=0; i<m_comments.size(); ++i)
		{
			res->AddComment(m_comments[i].c_str());
		}

		//get data type
		std::string className;
		ParseScriptClassName(className);
		res->SetType(className.c_str());

		//get variable name
		Accept(TOKEN_IDENTIFIER);
		res->SetName(PrevToken().GetSpelling());

		if (CurToken().GetType() == TOKEN_SEMICOLON)
		{
			//get ;
			m_comments.clear();
			Accept(TOKEN_SEMICOLON);
			return res;
		}

		// get =
		Accept(TOKEN_ASSIGN);

		// get initializer
		ExpressionSrcCPtr expr = ParseExpression();
		res->SetExpression(expr);

		//get ;
		Accept(TOKEN_SEMICOLON);

		return res;
	}

	//--------------------------------------------------------------------------------
	FunctionSrcCPtr Parser::ParseFunction()
	{
		FunctionSrcCPtr res = new FunctionSrc();
		res->SetLine(CurToken().GetLine());

		//add function comments
		for (uint32 i=0; i<m_comments.size(); ++i)
		{
			res->AddComment(m_comments[i].c_str());
		}

		//get return type
		std::string returnClass;
		ParseScriptClassName(returnClass);
		if (CurToken().GetType() == TOKEN_OPEN_BRACKET)
		{
			//constructor
			if (strcmp(returnClass.c_str(), m_cpScriptSource->GetName()) != 0)
				throw CompilerException(FORMAT("No return value specified.  Class %s, line %u.", m_cpScriptSource->GetName(), CurToken().GetLine()));
			res->SetReturnType("void");
			res->SetName(returnClass.c_str());
			res->SetConstructor();
		}
		else
		{
			res->SetReturnType(returnClass.c_str());

			//get funcion name
			Accept(TOKEN_IDENTIFIER);
			res->SetName(PrevToken().GetSpelling());
		}

		//opening '('
		Accept(TOKEN_OPEN_BRACKET);

		// function parameters
		if (CurToken().GetType() != TOKEN_CLOSE_BRACKET)
		{
			//get first parameter type
			std::string paramClass;
			ParseScriptClassName(paramClass);

			//get first parameter name
			Accept(TOKEN_IDENTIFIER);
			res->PushParameter(paramClass.c_str(), PrevToken().GetSpelling(), PrevToken().GetLine());

			//get the rest of the parameters
			while (CurToken().GetType() == TOKEN_COMMA)
			{
				Accept(TOKEN_COMMA);

				//get parameter type
				std::string paramClass;
				ParseScriptClassName(paramClass);

				//get parameter name
				Accept(TOKEN_IDENTIFIER);
				res->PushParameter(paramClass.c_str(), PrevToken().GetSpelling(), PrevToken().GetLine());
			}
		}

		//closing ')'
		Accept(TOKEN_CLOSE_BRACKET);

		//native function
		if (CurToken().GetType() == TOKEN_SEMICOLON)
		{
			if (!m_cpScriptSource->IsNative())
			{
				throw CompilerException(FORMAT("Native function \"%s\" found in non-native class.  Class %s, line %u.",
					res->GetName(), m_cpScriptSource->GetName(), CurToken().GetLine()));
			}

			m_comments.clear();
			Accept(TOKEN_SEMICOLON);
			res->SetNative(true);
			return res;
		}

		//NON_NATIVE FUNCTION BODY
		res->SetNative(false);

		//opening '{'
		Accept(TOKEN_OPEN_CURLY_BRACKET);

		//get statements
		if (CurToken().GetType() != TOKEN_CLOSE_CURLY_BRACKET)
		{
			//get data members
			while (IsLocalData())
			{
				DataSrcCPtr data = ParseLocalData();
				res->AddLocalDataMember(data);
			}

			//parse base constructor if needed
			if (res->IsConstructor() && strlen(m_cpScriptSource->GetSuper()) > 0)
			{
				FunctionCallSrcCPtr fncCall = ParseFunctionCall();
				if (strcmp(fncCall->GetName(), "super") != 0)
				{
					throw CompilerException(FORMAT("Super's constructor has to be the first statement in the constructor.  Class %s, line %u.",
						m_cpScriptSource->GetName(), fncCall->GetLine()));
				}
				res->SetBaseConstructor(fncCall);
			}

			//get statement block
			StatementSrcCPtr statement = ParseStatementBlock();
			res->SetStatement(statement);
		}

		//closing '}'
		m_comments.clear();
		Accept(TOKEN_CLOSE_CURLY_BRACKET);

		return res;
	}

	//--------------------------------------------------------------------------------
	StatementSrcCPtr Parser::ParseStatement()
	{
		uint32 line = CurToken().GetLine();

		switch (CurToken().GetType())
		{
		case TOKEN_WHILE:
			{
				Accept(TOKEN_WHILE);

				//opening '('
				Accept(TOKEN_OPEN_BRACKET);

				//expression
				ExpressionSrcCPtr cpExpression = ParseExpression();

				//closing ')'
				Accept(TOKEN_CLOSE_BRACKET);

				//statement block
				StatementSrcCPtr statement = ParseStatement();

				StatementSrcCPtr res(new StWhile(cpExpression, statement));
				res->SetLine(line);
				return res;
			}
			break;

		case TOKEN_IF:
			{
				Accept(TOKEN_IF);

				//opening '('
				Accept(TOKEN_OPEN_BRACKET);

				//expression
				ExpressionSrcCPtr cpExpression = ParseExpression();

				//closing ')'
				Accept(TOKEN_CLOSE_BRACKET);

				//statement block
				StatementSrcCPtr trueStatement = ParseStatement();

				//check for else
				StatementSrcCPtr falseStatement = 0;
				if (CurToken().GetType() == TOKEN_ELSE)
				{
					Accept(TOKEN_ELSE);

					//statement block
					falseStatement = ParseStatement();
				}

				StatementSrcCPtr res(new StIf(cpExpression, trueStatement, falseStatement));
				res->SetLine(line);
				return res;
			}
			break;

		case TOKEN_RETURN:
			{
				Accept(TOKEN_RETURN);

				ExpressionSrcCPtr retVal = 0;
				if (CurToken().GetType() == TOKEN_SEMICOLON)
				{
					//no return value
					Accept(TOKEN_SEMICOLON);
				}
				else
				{
					retVal = ParseExpression();

					// ';'
					Accept(TOKEN_SEMICOLON);
				}

				StatementSrcCPtr res(new StReturn(retVal));
				res->SetLine(line);
				return res;
			}
			break;

		case TOKEN_OPEN_CURLY_BRACKET:
			{
				Accept(TOKEN_OPEN_CURLY_BRACKET);
				StatementSrcCPtr res = ParseStatementBlock();

				Accept(TOKEN_CLOSE_CURLY_BRACKET);

				return res;
			}
			break;

		case TOKEN_SUPER:
			{
				//super function call
				FunctionCallSrcCPtr function = ParseFunctionCall();

				while (CurToken().GetType() == TOKEN_DOT)
				{
					FunctionCallSrcCPtr function2 = ParseFunctionCall();
					function->AddFncCall(function2);
				}

				StatementSrcCPtr res(new StFunctionCall(function));
				res->SetLine(line);

				Accept(TOKEN_SEMICOLON);

				return res;
			}
			break;

		case TOKEN_NEW:
			{
				//function call
				assert(IsFunctionCall());
				FunctionCallSrcCPtr function = ParseFunctionCall();

				while (CurToken().GetType() == TOKEN_DOT)
				{
					FunctionCallSrcCPtr function2 = ParseFunctionCall();
					function->AddFncCall(function2);
				}

				StatementSrcCPtr res(new StFunctionCall(function));
				res->SetLine(line);

				Accept(TOKEN_SEMICOLON);

				return res;
			}		
			break;

		case TOKEN_IDENTIFIER:
			{
				switch (NextToken().GetType())
				{
				case TOKEN_DOT:
				case TOKEN_OPEN_BRACKET:
					{
						//function call
						assert(IsFunctionCall());
						FunctionCallSrcCPtr function = ParseFunctionCall();

						while (CurToken().GetType() == TOKEN_DOT)
						{
							FunctionCallSrcCPtr function2 = ParseFunctionCall();
							function->AddFncCall(function2);
						}

						StatementSrcCPtr res(new StFunctionCall(function));
						res->SetLine(line);

						Accept(TOKEN_SEMICOLON);

						return res;
					}		
					break;

				case TOKEN_ASSIGN:
					{
						//assignment statement
						Accept(TOKEN_IDENTIFIER);
						std::string vname = PrevToken().GetSpelling();

						Accept(TOKEN_ASSIGN);

						ExpressionSrcCPtr cpExpression = ParseExpression();

						Accept(TOKEN_SEMICOLON);

						StatementSrcCPtr res(new StAssign(vname, cpExpression));
						res->SetLine(line);
						return res;
					}
					break;

				default:
					assert(false);
					throw CompilerException(FORMAT("Invalid statement.  Class %s, line %u.",
						m_cpScriptSource->GetName(), line));
					break;
				}
			}
			break;

		default:
			assert(false);
			throw CompilerException(FORMAT("Invalid statement.  Class %s, line %u.",
				m_cpScriptSource->GetName(), line));
		}

		assert(false);
		throw CompilerException(FORMAT("Invalid statement.  Class %s, line %u.",
			m_cpScriptSource->GetName(), line));
		return 0;
	}

	//--------------------------------------------------------------------------------
	StatementSrcCPtr Parser::ParseStatementBlock()
	{
		StBlockCPtr cpBlock(new StBlock());
		cpBlock->SetLine(CurToken().GetLine());

		while (CurToken().GetType() != TOKEN_CLOSE_CURLY_BRACKET)
		{
			StatementSrcCPtr cpStatement = ParseStatement();
			cpBlock->AddStatement(cpStatement);
		}

		return StatementSrcCPtr(cpBlock.get());
	}

	//--------------------------------------------------------------------------------
	ExpressionSrcCPtr Parser::ParseExpression()
	{
		ExpressionSrcCPtr res = new ExpressionSrc();
		res->SetLine(CurToken().GetLine());

		std::list<Token> stack;

		while (true)
		{
			if (CurToken().GetType() == TOKEN_SEMICOLON)
			{
				while (!stack.empty())
				{
					Token tok = stack.back();
					stack.pop_back();
					res->Push(tok);
				}

				return res;
			}
			else if (CurToken().GetType() == TOKEN_COMMA)
			{
				while (!stack.empty())
				{
					Token tok = stack.back();
					stack.pop_back();
					res->Push(tok);
				}

				return res;
			}
			else if (CurToken().GetType() == TOKEN_EOF)
			{
				assert(false);
				throw CompilerException(FORMAT("Unexpected EOF while parsing an expression.  Class %s, line %u.",
					m_cpScriptSource->GetName(), res->GetLine()));
			}
			else if (IsFunctionCall())
			{
				FunctionCallSrcCPtr function = ParseFunctionCall();

				while (CurToken().GetType() == TOKEN_DOT)
				{
					assert(IsFunctionCall());
					FunctionCallSrcCPtr function2 = ParseFunctionCall();
					function->AddFncCall(function2);
				}

				res->Push(function);
			}
			else if (CurToken().GetType() == TOKEN_CLOSE_BRACKET)
			{
				if (stack.empty())
				{
					return res;
				}
				else
				{
					Token topOfStack = stack.back();
					stack.pop_back();

					while (topOfStack.GetType() != TOKEN_OPEN_BRACKET)
					{
						if (stack.empty())
						{
							res->Push(topOfStack);
							return res;
						}
						else
						{
							res->Push(topOfStack);
							topOfStack = stack.back();
							stack.pop_back();
						}
					}
				}
				Accept(CurToken().GetType());
			}
			else if (CurToken().GetType() == TOKEN_OPEN_BRACKET)
			{
				stack.push_back(CurToken());
				Accept(CurToken().GetType());
			}
			else if (IsOperator(CurToken().GetType()))
			{
				if (!stack.empty())
				{
					Token topOfStack = stack.back();
					while (topOfStack.GetType() != TOKEN_ERROR && GetOperatorPriority(CurToken().GetType()) <= GetOperatorPriority(topOfStack.GetType()))
					{
						stack.pop_back();
						res->Push(topOfStack);

						if (stack.empty())
							topOfStack = Token(TOKEN_ERROR, "", 0, 0);
						else
							topOfStack = stack.back();
					}
				}

				stack.push_back(CurToken());
				Accept(CurToken().GetType());
			}
			else
			{
				res->Push(CurToken());
				Accept(CurToken().GetType());
			}
		}

		assert(false);
		throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.",
			m_cpScriptSource->GetName(), res->GetLine()));
		return 0;
	}

	//--------------------------------------------------------------------------------
	FunctionCallSrcCPtr Parser::ParseFunctionCall()
	{
		FunctionCallSrcCPtr res = new FunctionCallSrc();
		res->SetLine(CurToken().GetLine());

		bool superConstructor = false;
		if (CurToken().GetType() == TOKEN_SUPER)
		{
			Accept(TOKEN_SUPER);

			if (CurToken().GetType() == TOKEN_DOT)
			{
				Accept(TOKEN_DOT);
				res->SetSuper();
			}
			else
			{
				res->SetName("super");
				superConstructor = true;
			}
		}
		else if (CurToken().GetType() == TOKEN_IDENTIFIER && NextToken().GetType() == TOKEN_DOT)
		{
			Accept(TOKEN_IDENTIFIER);
			res->SetInstance(PrevToken().GetSpelling());
			Accept(TOKEN_DOT);
		}
		else if (CurToken().GetType() == TOKEN_NEW)
		{
			Accept(TOKEN_NEW);
			res->SetNew();
		}

		if (res->IsNew())
		{
			std::string className;
			ParseScriptClassName(className);
			res->SetName(className.c_str());
		}
		else if (superConstructor)
		{
		}
		else
		{
			//get name
			Accept(TOKEN_IDENTIFIER);
			res->SetName(PrevToken().GetSpelling());
		}

		//opening '('
		Accept(TOKEN_OPEN_BRACKET);

		//get parameters
		while (CurToken().GetType() != TOKEN_CLOSE_BRACKET)
		{
			//get expression
			ExpressionSrcCPtr expr = ParseExpression();
			res->PushParameter(expr);

			if (CurToken().GetType() != TOKEN_CLOSE_BRACKET)
				Accept(TOKEN_COMMA);
		}

		Accept(TOKEN_CLOSE_BRACKET);

		return res;
	}

	//--------------------------------------------------------------------------------
	int32 Parser::GetOperatorPriority(uint32 tokenType) const
	{
		int32 prio = -1;
		switch (tokenType)
		{
		case TOKEN_OPEN_BRACKET:
			prio = 0;
			break;

		case TOKEN_OR:
			prio = 1;
			break;

		case TOKEN_AND:
			prio = 2;
			break;

		case TOKEN_LT:
			prio = 3;
			break;

		case TOKEN_LTEQ:
			prio = 4;
			break;

		case TOKEN_GT:
			prio = 5;
			break;

		case TOKEN_GTEQ:
			prio = 6;
			break;

		case TOKEN_EQUALS:
			prio = 7;
			break;

		case TOKEN_NOT_EQUALS:
			prio = 8;
			break;

		case TOKEN_PLUS:
			prio = 9;
			break;

		case TOKEN_MINUS:
			prio = 10;
			break;

		case TOKEN_MODULO:
			prio = 11;
			break;

		case TOKEN_MULTIPLY:
			prio = 12;
			break;

		case TOKEN_DIVIDE:
			prio = 13;
			break;

		case TOKEN_UNARY_MINUS:
			prio = 14;
			break;

		case TOKEN_NOT:
			prio = 15;
			break;

		default:
			assert(false);
			prio = -1;
			break;
		}

		return prio;
	}

	//--------------------------------------------------------------------------------
	bool Parser::IsFunctionCall() const
	{
		if (CurToken().GetType() == TOKEN_IDENTIFIER && NextToken().GetType() == TOKEN_OPEN_BRACKET)
			return true;

		if (CurToken().GetType() == TOKEN_DOT && NextToken().GetType() == TOKEN_IDENTIFIER && NextNextToken().GetType() == TOKEN_OPEN_BRACKET)
			return true;

		if (CurToken().GetType() == TOKEN_IDENTIFIER && NextToken().GetType() == TOKEN_DOT && NextNextToken().GetType() == TOKEN_IDENTIFIER && NextNextNextToken().GetType() == TOKEN_OPEN_BRACKET)
			return true;

		if (CurToken().GetType() == TOKEN_SUPER && NextToken().GetType() == TOKEN_DOT && NextNextToken().GetType() == TOKEN_IDENTIFIER && NextNextNextToken().GetType() == TOKEN_OPEN_BRACKET)
			return true;

		if (CurToken().GetType() == TOKEN_SUPER && NextToken().GetType() == TOKEN_OPEN_BRACKET)
			return true;

		if (CurToken().GetType() == TOKEN_NEW)
			return true;

		return false;
	}

	//--------------------------------------------------------------------------------
	bool Parser::IsOperator(uint32 tokenType) const
	{
		bool res = false;

		switch (tokenType)
		{
		case TOKEN_OPEN_BRACKET:
		case TOKEN_OR:
		case TOKEN_AND:
		case TOKEN_LT:
		case TOKEN_LTEQ:
		case TOKEN_GT:
		case TOKEN_GTEQ:
		case TOKEN_EQUALS:
		case TOKEN_NOT_EQUALS:
		case TOKEN_PLUS:
		case TOKEN_MINUS:
		case TOKEN_MODULO:
		case TOKEN_MULTIPLY:
		case TOKEN_DIVIDE:
		case TOKEN_UNARY_MINUS:
		case TOKEN_NOT:
			res = true;
			break;

		default:
			res = false;
			break;
		}

		return res;
	}

	//--------------------------------------------------------------------------------
	void Parser::GetCurrentLocation(uint32& line, uint32& col) const
	{
		line = CurToken().GetLine();
		col = CurToken().GetCol();
	}

	//--------------------------------------------------------------------------------
	bool Parser::IsLocalData() const
	{
		TokenList::const_iterator itt = m_curTokenItt;
		if (itt == m_tokens.end())
			return false;

		//read class name
		if ((*itt).GetType() != TOKEN_IDENTIFIER)
			return false;
		++itt;
		if (itt == m_tokens.end())
			return false;
		while ((*itt).GetType() == TOKEN_DOT)
		{
			++itt;
			if (itt == m_tokens.end())
				return false;
			if ((*itt).GetType() != TOKEN_IDENTIFIER)
				return false;
			++itt;
			if (itt == m_tokens.end())
				return false;
		}

		//read var name
		if ((*itt).GetType() != TOKEN_IDENTIFIER)
			return false;
		++itt;
		if (itt == m_tokens.end())
			return false;

		//check for no initializers
		if ((*itt).GetType() == TOKEN_SEMICOLON	|| (*itt).GetType() == TOKEN_ASSIGN)
			return true;

		return false;
	}
}
