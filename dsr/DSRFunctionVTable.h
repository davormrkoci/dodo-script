#if !defined(DSR_FUNCTIONVTABLE_H_)
#define DSR_FUNCTIONVTABLE_H_

#include "DSRPlatform.h"
#include "DSRClassUtils.h"
#include "DSRMemory.h"
#include "DSRArray.h"

namespace dsr
{
	class FunctionImplementation;

	class FunctionVTable
	{
		DSR_NOCOPY(FunctionVTable)
	public:
		DSR_NEWDELETE(FunctionVTable)

		typedef Array<FunctionImplementation*> FunctionImplementationPtrArray;

		uint32 GetNumFunctions() const
		{
			return m_functions.size();
		}

		const FunctionImplementation* GetFunctionImplementationPtr(uint32 idx) const
		{
			return m_functions[idx];
		}

	private:
		FunctionImplementationPtrArray m_functions;
	};
}

#endif