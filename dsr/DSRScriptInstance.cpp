#include "DSRScriptInstance.h"
#include "DSRScriptManager.h"

namespace dsr
{
	ScriptInstance::ScriptInstance(const ScriptClass* pClass)
	: m_scriptClass(pClass), m_instanceData(0), m_handleOwner()
	{
		DSR_ASSERT(pClass);

		m_instanceData = new int32[m_scriptClass->GetNumData()];
		for (uint32 i=0; i<m_scriptClass->GetNumData(); ++i)
		{
			if (m_scriptClass->GetDataTypePtr(i)->GetVMDataType().IsNative())
			{
				ScriptInstanceHandle* pHandle = new ScriptInstanceHandle(0);
				pHandle->AddReference();
				m_instanceData[i] = *((int32*)(&pHandle));
			}
			else
			{
				m_instanceData[i] = 0;
			}
		}

		m_handleOwner.SetHandle(this);

		ScriptManagerPtr()->Add(this);
	}

	ScriptInstance::~ScriptInstance()
	{
		ScriptManagerPtr()->Remove(this);

		for (uint32 i=0; i<m_scriptClass->GetNumData(); ++i)
		{
			if (m_scriptClass->GetDataTypePtr(i)->GetVMDataType().IsNative())
			{
				ScriptInstanceHandle* pHandle = *((ScriptInstanceHandle**)(&m_instanceData[i]));
				pHandle->RemoveReference();
			}
		}
	}
}