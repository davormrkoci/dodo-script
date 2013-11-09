#include "DSRScriptManager.h"
#include "DSRScriptInstance.h"
#include "DSRScriptFactory.h"

namespace dsr
{
	ScriptManager* ScriptManager::m_pScriptManager = 0;

	ScriptManager::ScriptManager()
	{
	}

	ScriptManager::~ScriptManager()
	{
		//delete script instances
		while (!m_scriptInsts.empty())
		{
			ScriptInstance* pInst = m_scriptInsts.front();
			delete pInst;
		}

		//delete classes
		while (!m_scriptClasses.empty())
		{
			ScriptClass* pClass = m_scriptClasses.front();
			delete pClass;
		}

		//delete factories
		while (!m_scriptFactories.empty())
		{
			ScriptFactory* pFact = m_scriptFactories.front();
			delete pFact;
		}
	}

	void ScriptManager::Create()
	{
		if (!m_pScriptManager)
			m_pScriptManager = new ScriptManager();
	}

	void ScriptManager::Destroy()
	{
		if (m_pScriptManager)
			delete m_pScriptManager;
	}
}