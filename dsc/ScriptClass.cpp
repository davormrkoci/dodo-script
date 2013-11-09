#include <stdarg.h>
#include "ScriptClass.h"
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

	const FunctionDeclaration* ScriptClassDeclaration::GetFunctionDeclarationPtr(uint32 idx) const
	{
		assert(idx < GetNumFunctions());

		if (!m_superName.empty())
		{
			uint32 numSuperIdx = 0;
			const ScriptClassDeclaration* pSuper = CompilerPtr()->GetScriptClassDeclarationPtr(m_superName.c_str());
			numSuperIdx = pSuper->GetNumFunctions();
			if (idx < numSuperIdx)
				return pSuper->GetFunctionDeclarationPtr(idx);
			else
				return m_funcDecls[idx - numSuperIdx];
		}
		else
			return m_funcDecls[idx];
	}

	const FunctionDeclaration* ScriptClassDeclaration::GetConstructorFunctionDeclarationPtr(uint32 idx) const
	{
		assert(idx < GetNumConstructors());
		return m_ctorDecls[idx];
	}

	int32 ScriptClassDeclaration::GetConstructorIndex(const DataDeclarationArray& params) const
	{
		for (uint32 i=0; i<GetNumConstructors(); ++i)
		{
			const FunctionDeclaration* pCon = GetConstructorFunctionDeclarationPtr(i);
			if (pCon->GetNumParameters() != params.size())
				continue;

			bool ok = true;
			for (uint32 j=0; j<pCon->GetNumParameters() && ok; ++j)
			{
				const DataDeclaration* pParam = pCon->GetParameterDataDeclarationPtr(j);

				if (pParam->GetType() != params[j].GetType())
				{
					ok = false;
					continue;
				}

				if (strcmp(pParam->GetNativeType(), params[j].GetNativeType()) != 0)
				{
					ok = false;
					continue;
				}
			}

			if (ok)
				return i;
		}

		return -1;
	}

	int32 ScriptClassDeclaration::GetFunctionIndex(const char* funcName) const
	{
		uint32 numSuperIdx = 0;
		const ScriptClassDeclaration* pSuper = 0;
		if (!m_superName.empty())
		{
			pSuper = CompilerPtr()->GetScriptClassDeclarationPtr(m_superName.c_str());
			numSuperIdx = pSuper->GetNumFunctions();
		}

		for (uint32 i=0; i<m_funcDecls.size(); ++i)
		{
			if (strcmp(funcName, m_funcDecls[i]->GetName()) == 0)
				return i + numSuperIdx;
		}

		if (pSuper)
			return pSuper->GetFunctionIndex(funcName);

		return -1;
	}

	uint32 ScriptClassDeclaration::GetNumFunctions() const
	{
		if (m_superName.empty())
			return (uint32) m_funcDecls.size();

		const ScriptClassDeclaration* pSuper = CompilerPtr()->GetScriptClassDeclarationPtr(m_superName.c_str());
		return (uint32) (pSuper->GetNumFunctions() + m_funcDecls.size());
	}

	uint32 ScriptClassDeclaration::GetNumData() const
	{
		if (m_superName.empty())
			return (uint32) m_dataDecls.size();

		const ScriptClassDeclaration* pSuper = CompilerPtr()->GetScriptClassDeclarationPtr(m_superName.c_str());
		return (uint32) (pSuper->GetNumData() + m_dataDecls.size());
	}

	const DataDeclaration* ScriptClassDeclaration::GetDataDeclarationPtr(uint32 idx) const
	{
		assert(idx < GetNumData());
		if (!m_superName.empty())
		{
			uint32 numSuperIdx = 0;
			const ScriptClassDeclaration* pSuper = CompilerPtr()->GetScriptClassDeclarationPtr(m_superName.c_str());
			numSuperIdx = pSuper->GetNumData();
			if (idx < numSuperIdx)
				return pSuper->GetDataDeclarationPtr(idx);
			else
				return m_dataDecls[idx - numSuperIdx];
		}
		else
			return m_dataDecls[idx];
	}

	void ScriptClassDeclaration::RemoveNonSuperFunctionDeclaration(uint32 idx)
	{
		assert(idx < GetNumNonSuperFunctions());
		FunctionDeclarationCPtrArray funcDecls = m_funcDecls;
		m_funcDecls.clear();
		m_funcDecls.reserve(m_funcDecls.size()-1);
		for (uint32 i=0; i<funcDecls.size(); ++i)
		{
			if (i != idx)
				m_funcDecls.push_back(funcDecls[i]);
		}
	}

	void ScriptClass::CreateFile(std::vector<uint8>& file) const
	{
	}

	uint32 FunctionImplementation::AddNewClassName(const char* name)
	{
		for (uint32 i=0; i<m_newClasses.size(); ++i)
		{
			if (strcmp(name, m_newClasses[i].c_str()) == 0)
				return i;
		}

		m_newClasses.push_back(name);
		return (uint32) m_newClasses.size() - 1;
	}
}