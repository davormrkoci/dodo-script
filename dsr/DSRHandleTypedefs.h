#if !defined(DSR_HANDLETYPEDEFS_H_)
#define DSR_HANDLETYPEDEF_H_

#include "DSRPlatform.h"
#include "DSRHandle.h"

namespace dsr
{
	class ScriptInstance;

	typedef Handle<ScriptInstance> ScriptInstanceHandle;
	typedef CountedPtr<ScriptInstanceHandle> ScriptInstanceHandleCPtr;
}

#endif