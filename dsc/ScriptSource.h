#if !defined(DSC_SCRIPTSOURCE_H_)
#define DSC_SCRIPTSOURCE_H_

#include <list>
#include <vector>
#include "Token.h"
#include "CountedPtr.h"
#include "ClassUtils.h"

namespace dsc
{
	class ExpressionSrc;
	class FunctionCallSrc;
	class StatementSrc;
	class DataSrc;
	class FunctionSrc;
	class ScriptSource;
	class StWhile;
	class StIf;
	class StReturn;
	class StFunctionCall;
	class StAssign;
	class StBlock;

	typedef CountedPtr<ExpressionSrc> ExpressionSrcCPtr;
	typedef CountedPtr<FunctionCallSrc> FunctionCallSrcCPtr;
	typedef CountedPtr<StatementSrc> StatementSrcCPtr;
	typedef CountedPtr<DataSrc> DataSrcCPtr;
	typedef CountedPtr<FunctionSrc> FunctionSrcCPtr;
	typedef CountedPtr<ScriptSource> ScriptSourceCPtr;
	typedef CountedPtr<StBlock> StBlockCPtr;

	template <class T>
	const T& GetListElement(const std::list<T>& listA, uint32 idx)
	{
		assert(idx < listA.size());
		std::list<T>::const_iterator it = listA.begin();
		for (uint32 i=0; i<idx; ++i)
			++it;
		return *it;
	}

	//-------------------------------------------------------------------------------------
	class ExpressionSrc : public CountedResource
	{
		DSC_NOCOPY(ExpressionSrc)
	public:
		class ExpressionMember
		{
		public:
			ExpressionMember();
			ExpressionMember(const Token& token);
			ExpressionMember(FunctionCallSrc* fncCall);

			const FunctionCallSrc* GetFunctionCallSrcPtr() const { return m_fncCall; }
			Token GetToken() const { return m_token; }

		private:
			Token m_token;
			FunctionCallSrcCPtr m_fncCall;
		};
		typedef std::list<ExpressionMember> ExpressionMemberList;
		friend const std::string ToString(const ExpressionSrc& s);

		ExpressionSrc() { m_line = 0; }
		void Push(const Token& tok);
		void Push(FunctionCallSrc* fnc);
		void SetLine(uint32 line);
		uint32 GetLine() const;
		uint32 GetNumExpressionMembers() const { return (uint32) m_members.size(); }
		const ExpressionMember* GetExpressionMemberPtr(uint32 idx) const { return &GetListElement(m_members, idx); }

	private:
		virtual const std::string ToString() const;

	private:
		ExpressionMemberList m_members;
		uint32 m_line;
	};

	//-------------------------------------------------------------------------------------
	class FunctionCallSrc : public CountedResource
	{
		DSC_NOCOPY(FunctionCallSrc)
	public:
		typedef std::list<ExpressionSrcCPtr> ExpressionSrcCPtrList;
		friend const std::string ToString(const FunctionCallSrc& s);

		FunctionCallSrc() { m_super = false; m_new = false; m_line = 0; }
		void SetName(const char* name);
		void PushParameter(ExpressionSrc* expr);
		void SetLine(uint32 line);
		void SetSuper() { m_super = true; }
		void SetInstance(const char* instanceName) { m_instanceName = instanceName; }
		void SetNew() { m_new = true; }
		void AddFncCall(FunctionCallSrc* fnc);

		const char* GetName() const { return m_name.c_str(); }
		uint32 GetLine() const { return m_line; }
		bool IsSuper() const { return m_super; }
		bool IsNew() const { return m_new; }
		bool IsBaseConstructor() const { return strcmp(m_name.c_str(), "super") == 0; }
		uint32 GetNumParameters() const { return (uint32) m_parameters.size(); }
		const ExpressionSrc* GetParameterExpressionSrcPtr(uint32 idx) const { return GetListElement(m_parameters, idx); }
		const char* GetVarName() const { return m_instanceName.c_str(); }
		bool IsVarCall() const { return !m_instanceName.empty(); }
		const FunctionCallSrc* GetNextFncCallSrcPtr() const { return m_nextFncCall; }

	private:
		virtual const std::string ToString() const;

	private:
		std::string m_name;
		ExpressionSrcCPtrList m_parameters;
		uint32 m_line;
		bool m_super;
		bool m_new;
		std::string m_instanceName;
		FunctionCallSrcCPtr m_nextFncCall;
	};

	//-------------------------------------------------------------------------------------
	class StatementSrcVisitor
	{
	public:
		virtual void VisitBlock(const StBlock& stBlock) = 0;
		virtual void VisitWhile(const StWhile& stWhile) = 0;
		virtual void VisitIf(const StIf& stIf) = 0;
		virtual void VisitAssign(const StAssign& stAssign) = 0;
		virtual void VisitFunctionCall(const StFunctionCall& stFncCall) = 0;
		virtual void VisitReturn(const StReturn& stReturn) = 0;
	};

	//-------------------------------------------------------------------------------------
	class StatementSrc : public CountedResource
	{
		DSC_NOCOPY(StatementSrc)
	public:
		friend const std::string ToString(const StatementSrc& s);

		// Types
		enum StatementType
		{
			ST_NULL = 0,
			ST_BLOCK,
			ST_IF,
			ST_WHILE,
			ST_ASSIGN,
			ST_RETURN,
			ST_FUNCTIONCALL
		};

		StatementSrc(StatementType type);
		virtual ~StatementSrc();
		virtual void Visit(StatementSrcVisitor& visitor) const = 0;
		void SetLine(uint32 line);
		uint32 GetLine() const { return m_line; }
		StatementType GetStatementType() const { return m_type; }

	protected:
		virtual const std::string ToString() const;

	private:
		StatementSrc();

	private:
		StatementType m_type;
		uint32 m_line;
	};

	//--------------------------------------------------------------------------------
	class StBlock : public StatementSrc
	{
		DSC_NOCOPY(StBlock)
	public:
		typedef std::list<StatementSrcCPtr> StatementList;

		StBlock();
		void AddStatement(StatementSrc* cpStatement);
		virtual void Visit(StatementSrcVisitor& visitor) const;
		uint32 GetNumStatements() const { return (uint32) m_statements.size(); }
		const StatementSrc* GetStatementSrcPtr(uint32 idx) const { return GetListElement(m_statements, idx); }

	private:
		virtual const std::string ToString() const;

	private:
		StatementList m_statements;
	};

	//--------------------------------------------------------------------------------
	class StWhile : public StatementSrc
	{
		DSC_NOCOPY(StWhile)
	public:
		StWhile(ExpressionSrc* condition, StatementSrc* statement);
		virtual void Visit(StatementSrcVisitor& visitor) const;
		const ExpressionSrc* GetConditionExpressionSrcPtr() const { return m_condition; }
		const StatementSrc* GetStatementSrcPtr() const { return m_statement; }

	private:
		virtual const std::string ToString() const;

	private:
		ExpressionSrcCPtr m_condition;
		StatementSrcCPtr m_statement;
	};
	
	//--------------------------------------------------------------------------------
	class StIf : public StatementSrc
	{
		DSC_NOCOPY(StIf)
	public:
		StIf(ExpressionSrc* condition, StatementSrc* trueStatement, StatementSrc* falseStatement);
		virtual void Visit(StatementSrcVisitor& visitor) const;
		const ExpressionSrc* GetConditionExpressionSrcPtr() const { return m_condition; }
		const StatementSrc* GetTrueStatementSrcPtr() const { return m_trueStatement; }
		const StatementSrc* GetFalseStatementSrcPtr() const { return m_falseStatement; }

	private:
		virtual const std::string ToString() const;

	private:
		ExpressionSrcCPtr m_condition;
		StatementSrcCPtr m_trueStatement;
		StatementSrcCPtr m_falseStatement;
	};

	//--------------------------------------------------------------------------------
	class StReturn : public StatementSrc
	{
		DSC_NOCOPY(StReturn)
	public:
		StReturn(ExpressionSrc* retVal);
		virtual void Visit(StatementSrcVisitor& visitor) const;
		const ExpressionSrc* GetReturnValueExpressionSrcPtr() const { return m_returnValue; }

	private:
		virtual const std::string ToString() const;

	private:
		ExpressionSrcCPtr m_returnValue;
	};

	//--------------------------------------------------------------------------------
	class StFunctionCall : public StatementSrc
	{
		DSC_NOCOPY(StFunctionCall)
	public:
		StFunctionCall(FunctionCallSrc* fncCall);
		virtual void Visit(StatementSrcVisitor& visitor) const;
		const FunctionCallSrc* GetFunctionCallSrcPtr() const { return m_fncCall; }

	private:
		virtual const std::string ToString() const;

	private:
		FunctionCallSrcCPtr m_fncCall;
	};

	//--------------------------------------------------------------------------------
	class StAssign : public StatementSrc
	{
		DSC_NOCOPY(StAssign)
	public:
		StAssign(std::string& vname, ExpressionSrc* expression);
		virtual void Visit(StatementSrcVisitor& visitor) const;
		const char* GetVariableName() const { return m_vname.c_str(); }
		const ExpressionSrc* GetExpressionSrcPtr() const { return m_expression; }

	private:
		virtual const std::string ToString() const;

	private:
		std::string m_vname;
		ExpressionSrcCPtr m_expression;
	};

	//-------------------------------------------------------------------------------------
	class DataSrc : public CountedResource
	{
		DSC_NOCOPY(DataSrc)
	public:
		typedef std::list<Token> TokenList;
		friend const std::string ToString(const DataSrc& s);

		DataSrc() { m_line = 0; }
		void SetType(const char* type);
		void SetName(const char* name);
		void SetLine(uint32 line);
		void SetExpression(ExpressionSrc* pExpr) { m_expr = pExpr; }

		const char* GetName() const { return m_vname.c_str(); }
		uint32 GetLine() const { return m_line; }
		void AddComment(const char* comment) { m_comment.push_back(comment); }
		const char* GetType() const { return m_type.c_str(); }
		const ExpressionSrc* GetExpressionSrcPtr() const { return m_expr; }

	private:
		const std::string ToString() const;

	private:
		std::string m_type;
		std::string m_vname;
		uint32 m_line;
		std::vector<std::string> m_comment;
		ExpressionSrcCPtr m_expr;
	};

	//-------------------------------------------------------------------------------------
	class FunctionParameterSrc
	{
	public:
		friend const std::string ToString(const FunctionParameterSrc& s);

		FunctionParameterSrc(const char* type, const char* name, uint32 line); 
		const char* GetName() const;
		const char* GetType() const;
		uint32 GetLine() const { return m_line; }

	private:
		const std::string ToString() const;

	private:
		std::string m_type;
		std::string m_name;
		uint32 m_line;
	};
	typedef std::list<FunctionParameterSrc> FunctionParameterList;

	//-------------------------------------------------------------------------------------
	class FunctionSrc : public CountedResource
	{
		DSC_NOCOPY(FunctionSrc)
	public:
		typedef std::list<DataSrcCPtr> DataSrcList;
		friend const std::string ToString(const FunctionSrc& s);

		FunctionSrc() { m_native = false; m_constructor = false; m_line = 0; }
		void SetReturnType(const char* type);
		void SetName(const char* name);
		void PushParameter(const char* type, const char* name, uint32 line);
		void SetNative(bool native);
		void SetStatement(StatementSrc* statement);
		void AddLocalDataMember(DataSrc* data);
		void SetLine(uint32 line);
		void SetConstructor() { m_constructor = true; }
		void SetBaseConstructor(FunctionCallSrc* statement) { m_baseConstructorCall = statement; }
		const FunctionCallSrc* GetBaseConstructor() const { return m_baseConstructorCall; }
		const char* GetName() const { return m_name.c_str(); }
		uint32 GetLine() const { return m_line; }
		void AddComment(const char* comment) { m_comment.push_back(comment); }
		const FunctionParameterSrc* GetFunctionParameterSrcPtr(uint32 i) const { return &GetListElement(m_parameters, i); }
		uint32 GetNumParameters() const { return (uint32) m_parameters.size(); }
		const char* GetReturnType() const { return m_returnType.c_str(); }
		bool IsNative() const { return m_native; }
		const StatementSrc* GetStatementSrcPtr() const { return m_statement; }
		uint32 GetNumLocals() const { return (uint32) m_locals.size(); }
		const DataSrc* GetLocalDataSrcPtr(uint32 i) const { return GetListElement(m_locals, i); }
		bool IsConstructor() const { return m_constructor; }

	private:
		const std::string ToString() const;

	private:
		std::string m_returnType;
		std::string m_name;
		FunctionParameterList m_parameters;
		bool m_native;
		StatementSrcCPtr m_statement;
		DataSrcList m_locals;
		uint32 m_line;
		std::vector<std::string> m_comment;
		bool m_constructor;
		FunctionCallSrcCPtr m_baseConstructorCall;
	};

	//-------------------------------------------------------------------------------------
	class ScriptSource : public CountedResource
	{
		DSC_NOCOPY(ScriptSource)
	public:
		typedef std::list<DataSrcCPtr> DataSrcList;
		typedef std::list<FunctionSrcCPtr> FunctionSrcList;
		typedef std::list<std::string> StringList;
		friend const std::string ToString(const ScriptSource& s);

		ScriptSource();
		void SetName(const char* name);
		const char* GetName() const;
		void SetSuper(const char* super);
		const char* GetSuper() const;
		void SetNative(bool native);
		bool IsNative() const;
		void AddDataMember(DataSrc* data);
		void AddFunction(FunctionSrc* fnc);
		void AddConstructor(FunctionSrc* fnc);
		void AddImportClass(const char* script);
		void AddComment(const char* comment) { m_comment.push_back(comment); }
		uint32 GetNumComments() const { return (uint32) m_comment.size(); }
		const char* GetComment(uint32 i) const { return m_comment[i].c_str(); }
		uint32 GetNumData() const { return (uint32) m_data.size(); }
		const DataSrc* GetDataSrcPtr(uint32 i) const { return GetListElement(m_data, i); }
		uint32 GetNumFunctions() const { return (uint32) m_functions.size(); }
		const FunctionSrc* GetFunctionSrcPtr(uint32 i) const { return GetListElement(m_functions, i); }
		uint32 GetNumImportClasses() const { return (uint32) m_importClasses.size(); }
		const char* GetImportClass(uint32 i) const { return GetListElement(m_importClasses, i).c_str(); }
		uint32 GetNumConstructors() const { return (uint32) m_constructors.size(); }
		const FunctionSrc* GetConstructorFunctionSrcPtr(uint32 i) const { return GetListElement(m_constructors, i); }

	private:
		const std::string ToString() const;

	private:
		std::vector<std::string> m_comment;
		std::string m_name;
		std::string	m_super;
		DataSrcList m_data;
		FunctionSrcList m_functions;
		FunctionSrcList m_constructors;
		StringList m_importClasses;
		bool m_native;
	};

	//-------------------------------------------------------------------------------------
}

#endif