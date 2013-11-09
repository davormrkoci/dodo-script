
#if !defined(DSR_DATATYPE_H_)
#define DSR_DATATYPE_H_

#include "DSRPlatform.h"
#include "DSRString.h"
#include "DSRVMDataType.h"
#include "DSRArray.h"

namespace dsr
{
	class DataType
	{
		DSR_NOCOPY(DataType)
	public:
		DSR_NEWDELETE(DataType)

		VMDataType GetVMDataType() const { return m_type; }
		const char* GetName() const { return m_name.c_str(); }

	private:
		DataType();		//not implemented

	private:
		String m_name;
		VMDataType m_type;
	};
	typedef Array<VMDataType> VMDataTypeArray;
}

#endif