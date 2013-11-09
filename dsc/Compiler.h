#if !defined(DSC_COMPILER_H_)
#define DSC_COMPILER_H_

#include "DSRVMDataType.h"
#include "ScriptSource.h"
#include "ScriptClass.h"

namespace dsc
{
	//-------------------------------------------------------------------------------------
	class CompilerException
	{
	public:
		CompilerException(const char* error) { m_error = error; }
		const char* GetError() const { return m_error.c_str(); }

	private:
		std::string m_error;
	};

	//-------------------------------------------------------------------------------------
	class Compiler : public StatementSrcVisitor
	{
		DSC_NOCOPY(Compiler)
	public:
		static void Create();
		static void Destroy();
		friend Compiler* CompilerPtr() { return Compiler::m_pInstance; }

		~Compiler();

		ScriptClassCPtr BuildScriptClass(const char* scriptName);
		void AddPath(const char* scriptPath) { m_paths.push_back(scriptPath); }
		const char* GetError() const { return m_error.c_str(); }
		const ScriptClassDeclaration* GetScriptClassDeclarationPtr(const char* className) const;

		virtual void VisitBlock(const StBlock& stBlock);
		virtual void VisitWhile(const StWhile& stWhile);
		virtual void VisitIf(const StIf& stIf);
		virtual void VisitAssign(const StAssign& stAssign);
		virtual void VisitReturn(const StReturn& stReturn);
		virtual void VisitFunctionCall(const StFunctionCall& fncCall);
	private:
		typedef std::list<std::string> StringList;
		typedef std::vector<dsr::VMBytecode> VMCodeBlock;
		typedef std::list<ScriptClassDeclarationCPtr> ScriptClassDeclarationCPtrList;
		typedef std::list<ScriptSourceCPtr> ScriptSourceCPtrList;

		class VarInfo
		{
			DSC_NOCOPY(VarInfo)
		public:
			enum DataLoc
			{
				DATALOC_SCRIPT,
				DATALOC_LOCAL,
				DATALOC_PARAMETER,
				DATALOC_MAX,
			};

			VarInfo() : m_loc(DATALOC_MAX), m_offset(0), m_type(dsr::VMDATATYPE_MAX), m_nativeType() {}
			DataLoc GetDataLoc() const { return m_loc; }
			uint32 GetDataOffset() const { return m_offset; }
			uint32 GetDataType() const { return m_type; }
			const char* GetDataNativeType() const { return m_nativeType.c_str(); }

			void Set(DataLoc dataLoc, uint32 offset, uint32 type, const char* nativeType)
			{
				m_loc = dataLoc;
				m_offset = offset;
				m_type = type;
				m_nativeType = nativeType;
			}

		private:
			DataLoc m_loc;
			uint32 m_offset;
			uint32 m_type;
			std::string m_nativeType;
		};

		enum CompilePass
		{
			COMPILEPASS_UNDEF,
			COMPILEPASS_LOADSCRIPTSOURCES,
			COMPILEPASS_BUILDDECLARATIONS1,
			COMPILEPASS_BUILDDECLARATIONS2,
			COMPILEPASS_BUILDFUNCTIONIMPLEMENTATIONS
		};

		Compiler();
		ScriptClassDeclarationCPtr BuildScriptClassDeclarationPass1(const ScriptSource& source) const;
		void BuildScriptClassDeclarationPass2(ScriptClassDeclaration* pDecl);
		ScriptClassCPtr BuildScriptClassImplementation(const char* scriptName);
		DataDeclarationCPtr BuildDataDeclaration(const DataSrc& source) const;
		DataDeclarationCPtr BuildDataDeclaration(const FunctionParameterSrc& source) const;
		FunctionDeclarationCPtr BuildFunctionDeclaration(const FunctionSrc& source) const;
		void IncStackSize() { ++m_curStackSize; if (m_curStackSize > m_maxStackSize) m_maxStackSize = m_curStackSize; }
		void DecStackSize() { assert(m_curStackSize > 0); --m_curStackSize; }
		void GetVarInfo(const char* vname, VarInfo& varInfo);
		void VisitFunctionCall(const FunctionCallSrc& fncCall, uint32& retValType, std::string& nativeRetType, const char* pushedType);
		void ExprPushValue(const Token& tok, uint32& type, std::string& nativeType);
		void ClearCurCode();
		bool IsA(const char* derived, const char* base) const;
		void Clear();
		void ClearPaths() { m_paths.clear(); }
		uint32 GetNumPaths() const { return (uint32) m_paths.size(); }
		const char* GetPath(uint32 idx) const { return GetListElement(m_paths, idx).c_str(); }
		ScriptSourceCPtr LoadScriptSource(const char* scriptName);
		void LoadAllScriptSources(const char* name);
		void BuildScriptClassDeclarationsPass1();
		void BuildScriptClassDeclarationsPass2();
		FunctionImplementationCPtr BuildFunctionImplementation(const FunctionSrc& source);
		virtual void VisitExpressionAndCheckRetTypes(const ExpressionSrc& expr, uint32 returnType, const char* nativeReturnType);
		virtual void VisitExpression(const ExpressionSrc& expr, uint32& retType, std::string& nativeRetType);
		void AssignToVariable(const VarInfo& varInfo);
		void CheckDataDeclarations(const ScriptClassDeclaration* pDecl);
		void CheckFunctionDeclarations(ScriptClassDeclaration* pDecl);
		void CheckConstructorDeclarations(ScriptClassDeclaration* pDecl);

	private:
		/// Current size of the stack.
		/// Used for calculating the maximum stack size needed for a function call.
		uint32 m_curStackSize;
		/// Maximum stack size.
		/// Used for calculating the maximum stack size needed for a function call.
		uint32 m_maxStackSize;
		VMCodeBlock m_curCode;
		std::string m_error;
		const ScriptClassDeclaration* m_pDeclaration;
		FunctionImplementation* m_pCurFuncImpl;
		uint32 m_curFuncIdx;
		uint32 m_curCtorIdx;
		ScriptClassDeclarationCPtrList m_declarationList;
		ScriptSourceCPtrList m_sources;
		StringList m_paths;
		CompilePass m_curCompilePass;

		static Compiler* m_pInstance;
	};
}

#endif