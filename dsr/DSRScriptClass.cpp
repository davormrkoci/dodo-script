#include "DSRScriptClass.h"
#include "DSRScriptFactory.h"
#include "DSRScriptInstance.h"

namespace dsr
{
	ScriptClass::~ScriptClass()
	{
	}

	int32 ScriptClass::GetFunctionVTableIndex(const char* functionName) const
	{
		for (int32 i=0; i<(int32)m_functionVTable.GetNumFunctions(); ++i)
		{
			const char* funcName = m_functionVTable.GetFunctionImplementationPtr(i)->GetFunctionDefinitionPtr()->GetName();
			if (stricmp(functionName, funcName) == 0)
				return i;
		}

		return -1;
	}

	int32 ScriptClass::GetDataTypeIndex(const char* dataName) const
	{
		for (int32 i=0; i<(int32) m_data.size(); ++i)
		{
			const char* dName = m_data[i].GetName();
			if (stricmp(dName, dataName) == 0)
			{
				if (m_super)
					return m_super->GetNumData() + i;
				else
					return i;
			}
		}

		if (m_super)
			return m_super->GetDataTypeIndex(dataName);
		else
			return -1;
	}

	const FunctionImplementation* ScriptClass::GetFunctionImplementationPtr(uint32 fnIdx) const
	{
		DSR_ASSERT(fnIdx >= 0 && fnIdx < m_functionVTable.GetNumFunctions());
		return m_functionVTable.GetFunctionImplementationPtr(fnIdx);
	}

	const FunctionDefinition* ScriptClass::GetFunctionDefinitionPtr(uint32 fnIdx) const
	{
		DSR_ASSERT(fnIdx >= 0 && fnIdx < m_functionVTable.GetNumFunctions());
		return m_functionVTable.GetFunctionImplementationPtr(fnIdx)->GetFunctionDefinitionPtr();
	}

	const DataType* ScriptClass::GetDataTypePtr(uint32 dataIdx) const
	{
		DSR_ASSERT(dataIdx >=0 && dataIdx < GetNumData());

		if (m_super && dataIdx < m_super->GetNumData())
			return m_super->GetDataTypePtr(dataIdx);
		else
			return &m_data[dataIdx];
	}

	void ScriptClass::SetNativeFunction(uint32 fncIdx, NativeFunctionImplementation::NativeScriptFunction* pFunc)
	{
		DSR_ASSERT(m_native);
		DSR_ASSERT(fncIdx >= 0 && fncIdx < GetNumFunctions());
		DSR_ASSERT(GetFunctionImplementationPtr(fncIdx)->IsNative());
		DSR_ASSERT(pFunc);
		NativeFunctionImplementation* pNFI = (NativeFunctionImplementation*) GetFunctionImplementationPtr(fncIdx);
		pNFI->SetNativeScriptFunction(pFunc);
	}

	void ScriptClass::SetNativeConstructor(uint32 cnIdx, NativeFunctionImplementation::NativeScriptFunction* pFunc)
	{
		DSR_ASSERT(m_native);
		DSR_ASSERT(cnIdx >= 0 && cnIdx < GetNumConstructors());
		DSR_ASSERT(GetConstructorImplementationPtr(cnIdx)->IsNative());
		DSR_ASSERT(pFunc);
		NativeFunctionImplementation* pNFI = (NativeFunctionImplementation*) GetConstructorImplementationPtr(cnIdx);
		pNFI->SetNativeScriptFunction(pFunc);
	}

	bool ScriptClass::IsA(const ScriptClass* pScriptType) const
	{
		if (pScriptType == this)
			return true;

		if (!m_super)
			return false;

		return m_super->IsA(pScriptType);
	}

	const ScriptClass* ScriptClass::GetClosestNativeClassPtr() const
	{
		if (m_native)
			return this;

		if (!m_super)
			return 0;

		return m_super->GetClosestNativeClassPtr();
	}

	ScriptInstance* ScriptClass::CreateInstance() const
	{
		ScriptInstance* retVal;
		const ScriptClass* pNative = GetClosestNativeClassPtr();

		if (pNative)
		{
			const ScriptFactory* pFactory = pNative->m_pFactory;
			DSR_ASSERT(pFactory);
			retVal = pFactory->CreateInstance(this);
		}
		else
		{
			retVal =  new ScriptInstance(this);
		}

		return retVal;
	}
}