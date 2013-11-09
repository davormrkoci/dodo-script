#if !defined(DSR_SCRIPTFACTORY_H_)
#define DSR_SCRIPTFACTORY_H_

#include "DSRPlatform.h"
#include "DSRClassUtils.h"
#include "DSRMemory.h"

namespace dsr
{
	class ScriptClass;
	class ScriptInstance;

	class ScriptFactory
	{
		DSR_NOCOPY(ScriptFactory)
	public:
		DSR_NEWDELETE(ScriptFactory)

		ScriptFactory();
		virtual ~ScriptFactory();

		virtual void Init(ScriptClass* pType) = 0;
		virtual void Close(ScriptClass* pType) = 0;
		virtual ScriptInstance* CreateInstance(const ScriptClass* pType) const = 0;
		virtual const char* GetName() const = 0;
	};
}

#endif