#if !defined(DSC_SCRIPTCLASS_H_)
#define DSC_SCRIPTCLASS_H_

#include <string>
#include <vector>
#include <list>
#include "CountedPtr.h"
#include "DSRVMInstruction.h"

namespace dsc
{
	class ScriptClass;
	class FunctionImplementation;
	class DataDeclaration;
	class FunctionDeclaration;
	class ScriptClassDeclaration;

	typedef CountedPtr<ScriptClass> ScriptClassCPtr;
	typedef CountedPtr<FunctionImplementation> FunctionImplementationCPtr;
	typedef CountedPtr<DataDeclaration> DataDeclarationCPtr;
	typedef CountedPtr<FunctionDeclaration> FunctionDeclarationCPtr;
	typedef CountedPtr<ScriptClassDeclaration> ScriptClassDeclarationCPtr;

	//---------------------------------------------------------
	class DataDeclaration : public CountedResource
	{
	public:
		const char* GetNativeType() const { return m_nativeType.c_str(); }
		uint32 GetType() const { return m_type; }
		const char* GetName() const { return m_name.c_str(); }
		void SetName(const char* name) { m_name = name; }
		void SetType(uint32 type) { m_type = type; }
		void SetNativeType(const char* nt) { m_nativeType = nt; }
		void SetLine(uint32 line) { m_line = line; }
		uint32 GetLine() const { return m_line; }

	private:
		std::string m_name;
		std::string m_nativeType;
		uint32 m_type;
		uint32 m_line;
	};

	//---------------------------------------------------------
	class FunctionDeclaration : public CountedResource
	{
	public:
		uint32 GetReturnType() const { return m_retType; }
		const char* GetNativeReturnType() const { return m_retNativeType.c_str(); }
		uint32 GetNumParameters() const { return (uint32) m_params.size(); }
		const DataDeclaration* GetParameterDataDeclarationPtr(uint32 idx) const { return m_params[idx];	}
		void SetScriptClassName(const char* name) { m_scriptClass = name; }
		const char* GetName() const { return m_name.c_str(); }
		void AddParameter(DataDeclaration* pParam) { m_params.push_back(pParam); }
		void SetName(const char* name) { m_name = name; }
		void SetReturnNativeType(const char* nt) { m_retNativeType = nt; }
		void SetReturnType(uint32 type) { m_retType = type; }
		void SetLine(uint32 line) { m_line = line; }
		uint32 GetLine() const { return m_line; }

	private:
		typedef std::vector<DataDeclarationCPtr> ParameterDeclarationCPtrArray;

	private:
		uint32 m_retType;
		std::string m_retNativeType;
		std::string m_scriptClass;
		std::string m_name;
		ParameterDeclarationCPtrArray m_params;
		uint32 m_line;
	};

	//---------------------------------------------------------
	class ScriptClassDeclaration : public CountedResource
	{
	public:
		typedef std::vector<DataDeclaration> DataDeclarationArray;

		const char* GetName() const { return m_name.c_str(); }
		const char* GetSuperClassName() const { return m_superName.c_str(); }
		const FunctionDeclaration* GetFunctionDeclarationPtr(uint32 idx) const;
		int32 GetConstructorIndex(const DataDeclarationArray& params) const;
		int32 GetFunctionIndex(const char* funcName) const;
		uint32 GetNumFunctions() const;
		uint32 GetNumConstructors() const { return (uint32) m_ctorDecls.size(); }
		uint32 GetNumData() const;
		const DataDeclaration* GetDataDeclarationPtr(uint32 i) const;
		const FunctionDeclaration* GetConstructorFunctionDeclarationPtr(uint32 idx) const;
		uint32 GetNumNonSuperFunctions() const { return (uint32) m_funcDecls.size(); }
		const FunctionDeclaration* GetNonSuperFunctionDeclarationPtr(uint32 idx) const { return m_funcDecls[idx]; }
		void RemoveNonSuperFunctionDeclaration(uint32 idx);

		void AddComment(const char* comment) { m_comments.push_back(comment); }
		void SetName(const char* name) { m_name = name; }
		void SetNative(bool native) { m_native = native; }
		void SetSuperClassName(const char* super) { m_superName = super; }
		void AddDataDeclaration(DataDeclaration* pData) { m_dataDecls.push_back(pData); }
		void AddFunctionDeclaration(FunctionDeclaration* pFunc) { m_funcDecls.push_back(pFunc); pFunc->SetScriptClassName(GetName()); }
		void AddConstructorDeclaration(FunctionDeclaration* pFunc) { m_ctorDecls.push_back(pFunc); pFunc->SetScriptClassName(GetName()); }

	private:
		typedef std::vector<std::string> StringArray;
		typedef std::vector<DataDeclarationCPtr> DataDeclarationCPtrArray;
		typedef std::vector<FunctionDeclarationCPtr> FunctionDeclarationCPtrArray;

	private:
		std::string m_name;
		std::string m_superName;
		bool m_native;
		StringArray m_comments;
		DataDeclarationCPtrArray m_dataDecls;
		FunctionDeclarationCPtrArray m_funcDecls;
		FunctionDeclarationCPtrArray m_ctorDecls;
	};

	//---------------------------------------------------------
	class FunctionImplementation : public CountedResource
	{
	public:
		typedef std::vector<dsr::VMBytecode> VMCodeBlock;

		uint32 GetNumLocals() const { return (uint32) m_locals.size(); }
		const DataDeclaration* GetLocalDataDeclarationPtr(uint32 i) const { return m_locals[i]; }
		void SetVMCodeBlock(const VMCodeBlock& code) { m_code = code; }
		void SetMaxStackSize(uint32 size) { m_maxStackSize = size; }
		void AddLocalDataDeclaration(DataDeclaration* pData) { m_locals.push_back(pData); }
		uint32 AddNewClassName(const char* name);

	private:
		typedef std::vector<DataDeclarationCPtr> DataDeclarationCPtrArray;
		typedef std::vector<std::string> StringArray;

	private:
		DataDeclarationCPtrArray m_locals;
		VMCodeBlock m_code;
		uint32 m_maxStackSize;
		StringArray m_newClasses;
	};

	//---------------------------------------------------------
	class ScriptClass : public CountedResource
	{
	public:
		void SetScriptDeclaration(const ScriptClassDeclaration* pDeclaration)
		{
			m_declaration = (ScriptClassDeclaration*) pDeclaration;
		}

		void AddFunctionImplementation(FunctionImplementation* pFuncImpl)
		{
			m_funcImpls.push_back(pFuncImpl);
		}

		void AddConstructorImplementation(FunctionImplementation* pFuncImpl)
		{
			m_ctorImpls.push_back(pFuncImpl);
		}

		void CreateFile(std::vector<uint8>& file) const;

	private:
		typedef std::vector<FunctionImplementationCPtr> FunctionImplementationCPtrArray;

	private:
		ScriptClassDeclarationCPtr m_declaration;
		FunctionImplementationCPtrArray m_funcImpls;
		FunctionImplementationCPtrArray m_ctorImpls;
	};
}

#endif