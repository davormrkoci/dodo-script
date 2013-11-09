#include <algorithm>
#include "ScriptSource.h"
#include "Compiler.h"

namespace dsc
{
	//--------------------------------------------------------------------------------
	class FindDataSrcByName
	{
	public:
		FindDataSrcByName(const char* name)
		{
			m_name = name;
		}

		bool operator()(DataSrc* data) const
		{
			return strcmp(data->GetName(), m_name.c_str()) == 0;
		}

	private:
		std::string m_name;
	};

	//--------------------------------------------------------------------------------
	class FindFunctionSrcByName
	{
	public:
		FindFunctionSrcByName(const char* name)
		{
			m_name = name;
		}

		bool operator()(FunctionSrc* fnc) const
		{
			return strcmp(fnc->GetName(), m_name.c_str()) == 0;
		}

	private:
		std::string m_name;
	};

	//--------------------------------------------------------------------------------
	class FindParameterSrcByName
	{
	public:
		FindParameterSrcByName(const char* name)
		{
			m_name = name;
		}

		bool operator()(const FunctionParameterSrc& param) const
		{
			return param.GetName() == m_name;
		}

	private:
		std::string m_name;
	};

	//--------------------------------------------------------------------------------
	ExpressionSrc::ExpressionMember::ExpressionMember()
	: m_token(TOKEN_ERROR, "", 0, 0), m_fncCall(0)
	{
	}

	ExpressionSrc::ExpressionMember::ExpressionMember(const Token& token)
	: m_token(token), m_fncCall(0)
	{
	}

	ExpressionSrc::ExpressionMember::ExpressionMember(FunctionCallSrc* fncCall)
	: m_token(TOKEN_ERROR, "", 0, 0), m_fncCall(fncCall)
	{
	}

	void ExpressionSrc::Push(const Token& tok)
	{
		m_members.push_back(tok);
	}

	void ExpressionSrc::SetLine(uint32 line)
	{
		m_line = line;
	}

	uint32 ExpressionSrc::GetLine() const
	{
		return m_line;
	}

	void ExpressionSrc::Push(FunctionCallSrc* fnc)
	{
		m_members.push_back(fnc);
	}

	const std::string ExpressionSrc::ToString() const
	{
		std::string res = "";

		uint32 i=0;
		for (ExpressionMemberList::const_iterator itt = m_members.begin(); itt != m_members.end(); ++itt)
		{
			const ExpressionMember& em = *itt;
			if (em.GetFunctionCallSrcPtr())
				res += dsc::ToString(*em.GetFunctionCallSrcPtr());
			else
				res += em.GetToken().GetSpelling();

			++i;
			if (i < m_members.size())
				res += " ";
		}

		return res;
	}

	const std::string ToString(const ExpressionSrc& s)
	{
		return s.ToString();
	}

	//--------------------------------------------------------------------------------
	void FunctionCallSrc::SetName(const char* name)
	{
		m_name = name;
	}

	void FunctionCallSrc::PushParameter(ExpressionSrc* expr)
	{
		m_parameters.push_back(expr);
	}

	void FunctionCallSrc::SetLine(uint32 line)
	{
		m_line = line;
	}

	const std::string FunctionCallSrc::ToString() const
	{
		std::string res = "";

		res += m_name;
		res += "(";

		uint32 i=0;
		for (ExpressionSrcCPtrList::const_iterator itt = m_parameters.begin(); itt != m_parameters.end(); ++itt)
		{
			res += dsc::ToString(**itt);
			++i;
			if (i < m_parameters.size())
				res += ", ";
		}
		res += ")";

		return res;
	}

	const std::string ToString(const FunctionCallSrc& s)
	{
		return s.ToString();
	}

	void FunctionCallSrc::AddFncCall(FunctionCallSrc* fnc)
	{
		if (!m_nextFncCall)
			m_nextFncCall = fnc;
		else
			m_nextFncCall->AddFncCall(fnc);
	}

	//--------------------------------------------------------------------------------
	StWhile::StWhile(ExpressionSrc* condition, StatementSrc* statement)
	: StatementSrc(ST_WHILE), m_condition(condition), m_statement(statement)
	{
	}

	void StWhile::Visit(StatementSrcVisitor& visitor) const
	{
		visitor.VisitWhile(*this);
	}

	const std::string StWhile::ToString() const
	{
		std::string res = "";

		res += "while (";
		res += dsc::ToString(*m_condition);
		res += ")\r\n";
		res += dsc::ToString(*m_statement);

		return res;
	}

	//--------------------------------------------------------------------------------
	StIf::StIf(ExpressionSrc* condition, StatementSrc* trueStatement, StatementSrc* falseStatement)
	: StatementSrc(ST_IF), m_condition(condition), m_trueStatement(trueStatement),
	  m_falseStatement(falseStatement)
	{
	}

	void StIf::Visit(StatementSrcVisitor& visitor) const
	{
		visitor.VisitIf(*this);
	}

	const std::string StIf::ToString() const
	{
		std::string res = "";

		res += "if (";
		res += dsc::ToString(*m_condition);
		res += ")\r\n";
		res += dsc::ToString(*m_trueStatement);
		if (m_falseStatement)
		{
			res += "\r\n";
			res += "else\r\n";
			res += dsc::ToString(*m_falseStatement);
		}

		return res;
	}

	//--------------------------------------------------------------------------------
	StReturn::StReturn(ExpressionSrc* retVal)
	: StatementSrc(ST_RETURN), m_returnValue(retVal)
	{
	}

	void StReturn::Visit(StatementSrcVisitor& visitor) const
	{
		visitor.VisitReturn(*this);
	}

	const std::string StReturn::ToString() const
	{
		std::string res = "";

		res += "return";
		if (m_returnValue)
		{
			res += " ";
			res += dsc::ToString(*m_returnValue);
			res += ";";
		}
		else
		{
			res += ";";
		}

		return res;
	}
	//--------------------------------------------------------------------------------
	StFunctionCall::StFunctionCall(FunctionCallSrc* fncCall)
	: StatementSrc(ST_FUNCTIONCALL), m_fncCall(fncCall)
	{
	}

	void StFunctionCall::Visit(StatementSrcVisitor& visitor) const
	{
		visitor.VisitFunctionCall(*this);
	}

	const std::string StFunctionCall::ToString() const
	{
		std::string res = "";

		res += dsc::ToString(*m_fncCall);
		res += ";";

		return res;
	}

	//--------------------------------------------------------------------------------
	StAssign::StAssign(std::string& vname, ExpressionSrc* expression)
	: StatementSrc(ST_ASSIGN), m_vname(vname), m_expression(expression)
	{
	}

	void StAssign::Visit(StatementSrcVisitor& visitor) const
	{
		visitor.VisitAssign(*this);
	}

	const std::string StAssign::ToString() const
	{
		std::string res = "";

		res += m_vname;
		res += " = ";
		res += dsc::ToString(*m_expression);
		res += ";";

		return res;
	}

	//--------------------------------------------------------------------------------
	void DataSrc::SetType(const char* type)
	{
		m_type = type;
	}

	void DataSrc::SetName(const char* name)
	{
		m_vname = name;
	}

	void DataSrc::SetLine(uint32 line)
	{
		m_line = line;
	}

	const std::string DataSrc::ToString() const
	{
		std::string res = "";

		res += m_type;
		res += " ";
		res += m_vname;
		res += ";";

		return res;
	}

	const std::string ToString(const DataSrc& s)
	{
		return s.ToString();
	}

	//--------------------------------------------------------------------------------
	FunctionParameterSrc::FunctionParameterSrc(const char* type, const char* name, uint32 line)
	: m_type(type), m_name(name), m_line(line)
	{
	}

	const char* FunctionParameterSrc::GetName() const
	{
		return m_name.c_str();
	}

	const char* FunctionParameterSrc::GetType() const
	{
		return m_type.c_str();
	}

	const std::string FunctionParameterSrc::ToString() const
	{
		std::string res = "";

		res += m_type;
		res += " ";
		res += m_name;

		return res;
	}

	const std::string ToString(const FunctionParameterSrc& s)
	{
		return s.ToString();
	}

	//--------------------------------------------------------------------------------
	void FunctionSrc::SetReturnType(const char* type)
	{
		m_returnType = type;
	}

	void FunctionSrc::SetName(const char* name)
	{
		m_name = name;
	}

	void FunctionSrc::SetLine(uint32 line)
	{
		m_line = line;
	}

	void FunctionSrc::PushParameter(const char* type, const char* name, uint32 line)
	{
		m_parameters.push_back(FunctionParameterSrc(type, name, line));
	}

	void FunctionSrc::SetNative(bool native)
	{
		m_native = native;
	}

	void FunctionSrc::SetStatement(StatementSrc* statement)
	{
		m_statement = statement;
	}

	void FunctionSrc::AddLocalDataMember(DataSrc* data)
	{
		m_locals.push_back(data);
	}

	const std::string FunctionSrc::ToString() const
	{
		std::string res = "";

		//return type
		res += m_returnType;

		res += " ";

		//name
		res += m_name;

		//parameters
		res += "(";
		uint32 i = 0;
		for (FunctionParameterList::const_iterator ittP = m_parameters.begin(); ittP != m_parameters.end(); ++ittP)
		{
			res += dsc::ToString(*ittP);
			++i;
			if (i < m_parameters.size())
				res += ", ";
		}
		res += ")";

		if (m_native)
		{
			res += ";";
		}
		else
		{
			res += "\r\n{\r\n";

			//locals
			for (DataSrcList::const_iterator ittD = m_locals.begin(); ittD != m_locals.end(); ++ittD)
			{
				res += dsc::ToString(**ittD);
				res += "\r\n";
			}

			//statement
			if (m_statement)
			{
				res += dsc::ToString(*m_statement);
				res += "\r\n";
			}

			res += "}";
		}

		return res;
	}

	const std::string ToString(const FunctionSrc& s)
	{
		return s.ToString();
	}

	//--------------------------------------------------------------------------------
	StatementSrc::StatementSrc(StatementType type)
	{
		m_type = type;
		m_line = 0;
	}

	StatementSrc::~StatementSrc()
	{
	}

	void StatementSrc::SetLine(uint32 line)
	{
		m_line = line;
	}

	const std::string StatementSrc::ToString() const
	{
		std::string res = "STATEMENT";

		return res;
	}

	const std::string ToString(const StatementSrc& s)
	{
		return s.ToString();
	}

	//--------------------------------------------------------------------------------
	StBlock::StBlock()
	: StatementSrc(ST_BLOCK)
	{
	}

	void StBlock::AddStatement(StatementSrc* cpStatement)
	{
		assert(cpStatement);
		m_statements.push_back(cpStatement);
	}

	void StBlock::Visit(StatementSrcVisitor& visitor) const
	{
		return visitor.VisitBlock(*this);
	}

	const std::string StBlock::ToString() const
	{
		std::string res = "";

		res += "{\r\n";
		for (StatementList::const_iterator itt = m_statements.begin(); itt != m_statements.end(); ++itt)
		{
			res += dsc::ToString(**itt);
			res += "\r\n";
		}
		res += "}";

		return res;
	}

	//--------------------------------------------------------------------------------
	ScriptSource::ScriptSource()
	{
		m_native = false;
	}

	void ScriptSource::SetName(const char* name)
	{
		m_name = name;
	}

	const char* ScriptSource::GetName() const
	{
		return m_name.c_str();
	}

	void ScriptSource::SetSuper(const char* super)
	{
		m_super = super;
	}

	const char* ScriptSource::GetSuper() const
	{
		return m_super.c_str();
	}

	void ScriptSource::SetNative(bool native)
	{
		m_native = native;
	}

	bool ScriptSource::IsNative() const
	{
		return m_native;
	}

	void ScriptSource::AddDataMember(DataSrc* data)
	{
		m_data.push_back(data);
	}

	void ScriptSource::AddFunction(FunctionSrc* fnc)
	{
		m_functions.push_back(fnc);
	}

	void ScriptSource::AddConstructor(FunctionSrc* fnc)
	{
		m_constructors.push_back(fnc);
	}

	void ScriptSource::AddImportClass(const char* import)
	{
		if (std::find(m_importClasses.begin(), m_importClasses.end(), std::string(import)) == m_importClasses.end())
		{
			m_importClasses.push_back(import);
		}
	}

	const std::string ScriptSource::ToString() const
	{
		std::string res = "";

		//include files
		for (StringList::const_iterator ittI = m_importClasses.begin(); ittI != m_importClasses.end(); ++ittI)
		{
			res += "import ";
			res += *ittI;
			res += "\r\n";
		}

		//name
		res += "script ";
		res += m_name;

		//super
		if (m_super.size() > 0)
		{
			res += " : ";
			res += m_super;
		}
		res += "\r\n";

		res += "{\r\n";

		//data
		for (DataSrcList::const_iterator ittD = m_data.begin(); ittD != m_data.end(); ++ittD)
		{
			res += dsc::ToString(**ittD);
			res += "\r\n";
		}
		res += "\r\n";

		//functions
		for (FunctionSrcList::const_iterator ittF = m_functions.begin(); ittF != m_functions.end(); ++ittF)
		{
			res += dsc::ToString(**ittF);
			res += "\r\n";
		}
		res += "\r\n";
		res += "\n}";

		return res;
	}

	const std::string ToString(const ScriptSource& s)
	{
		return s.ToString();
	}

	//--------------------------------------------------------------------------------
}