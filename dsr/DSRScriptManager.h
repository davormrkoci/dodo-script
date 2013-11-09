
#if !defined(DSR_SCRIPTMANAGER_H_)
#define DSR_SCRIPTMANAGER_H_

#include "DSRPlatform.h"
#include "DSRMemory.h"
#include "DSRClassUtils.h"
#include "DSRList.h"

namespace dsr
{
	class ScriptInstance;
	class ScriptFactory;
	class ScriptClass;

	class ScriptManager
	{
		DSR_NOCOPY(ScriptManager)
	public:
		DSR_NEWDELETE(ScriptManager)

		static void Create();
		static void Destroy();

		friend ScriptManager* ScriptManagerPtr()
		{
			return ScriptManager::m_pScriptManager;
		}

		/** Add ScriptClass.  Should only get called by ScriptClass */
		void Add(ScriptClass* pClass)
		{
			m_scriptClasses.push_front(pClass);
		}

		/** Remove ScriptClass.  Should only get called by ScriptClass */
		void Remove(ScriptClass* pClass)
		{
			m_scriptClasses.remove(pClass);
		}

		void Add(ScriptInstance* pInstance)
		{
			m_scriptInsts.push_front(pInstance);
		}

		void Remove(ScriptInstance* pInstance)
		{
			m_scriptInsts.remove(pInstance);
		}

		void Add(ScriptFactory* pFactory)
		{
			m_scriptFactories.push_front(pFactory);
		}

		void Remove(ScriptFactory* pFactory)
		{
			m_scriptFactories.remove(pFactory);
		}

		const ScriptClass* GetScriptClassPtr(const char* name) const;

	private:
		ScriptManager();

	private:
		static ScriptManager* m_pScriptManager;
		List<ScriptClass*> m_scriptClasses;
		List<ScriptInstance*> m_scriptInsts;
		List<ScriptFactory*> m_scriptFactories;
	};
}

#endif