
#if !defined(DSR_VMDATAVAL_H_)
#define DSR_VMDATAVAL_H_

#include "DSRPlatform.h"
#include "DSRBaseTypes.h"
#include "DSRHandleTypedefs.h"

namespace dsr
{
	union VMDataVal
	{
		int32 intVal;
		float32 floatVal;
		bool boolVal;
		ScriptInstanceHandle* nativeVal;
	};
}

#endif