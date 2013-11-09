
#if !defined(DSR_VMDATATYPE_H_)
#define DSR_VMDATATYPE_H_

#include "DSRPlatform.h"
#include "DSRBaseTypes.h"
#include "DSRClassUtils.h"
#include "DSRMemory.h"

namespace dsr
{
	class ScriptClass;

	enum
	{
		VMDATATYPE_NATIVE = 0,
		VMDATATYPE_FLOAT,
		VMDATATYPE_INT,
		VMDATATYPE_BOOL,
		VMDATATYPE_VOID,		//used by compiler only
		VMDATATYPE_MAX
	};
	typedef uint32 VMDataTypeEnum;

	class VMDataType
	{
	public:
		DSR_NEWDELETE(VMDataType)

		VMDataType()
		{
			m_data.type = VMDATATYPE_INT;
		}

		~VMDataType()
		{
		}

		explicit VMDataType(const ScriptClass* pType)
		{
			DSR_ASSERT(pType);
			DSR_ASSERT(*((VMDataTypeEnum*)&pType) <= VMDATATYPE_NATIVE
				|| *((VMDataTypeEnum*)&pType) >= VMDATATYPE_MAX);
			m_data.pType = pType;
		}

		explicit VMDataType(VMDataTypeEnum type)
		{
			DSR_ASSERT(type > VMDATATYPE_NATIVE && type < VMDATATYPE_MAX);
			m_data.type = type;
		}

		bool IsNative() const
		{
			return m_data.type <= VMDATATYPE_NATIVE	|| m_data.type >= VMDATATYPE_MAX;
		}

		VMDataTypeEnum GetVMDataTypeEnum() const
		{
			if (IsNative())
				return VMDATATYPE_NATIVE;

			return m_data.type;
		}

		const ScriptClass* GetScriptClassPtr() const
		{
			DSR_ASSERT(IsNative());
			return m_data.pType;
		}

		bool operator==(const VMDataType& rhs) const
		{
			if (IsNative())
				return m_data.pType == rhs.m_data.pType;
			else
				return m_data.type == rhs.m_data.type;
		}

		void Set(VMDataTypeEnum type)
		{
			DSR_ASSERT(type > VMDATATYPE_NATIVE && type < VMDATATYPE_MAX);
			m_data.type = type;
		}

		void Set(const ScriptClass* pType)
		{
			DSR_ASSERT(pType);
			m_data.pType = pType;
		}
		
	private:
		union
		{
			const ScriptClass* pType;
			VMDataTypeEnum type;
		} m_data;
	};
}

#endif