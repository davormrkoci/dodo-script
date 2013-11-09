// (c) 2004 DodoSoft Inc.

#include "DSRVMData.h"

namespace dsr
{
	VMData::VMData()
	: m_type(VMDATATYPE_INT), m_val()
	{
	}

	VMData::~VMData()
	{
		Clear();
	}

	VMData::VMData(const VMData& rhs)
	{
		if (rhs.m_type.IsNative())
		{
			m_val.nativeVal = rhs.m_val.nativeVal;
			m_val.nativeVal->AddReference();
		}
		else
		{
			m_val.intVal = m_val.intVal;
		}

		m_type = rhs.m_type;
	}

	VMData& VMData::operator=(const VMData& rhs)
	{
		if (this == &rhs)
			return *this;

		Clear();

		if (rhs.m_type.IsNative())
		{
			m_val.nativeVal = rhs.m_val.nativeVal;
			m_val.nativeVal->AddReference();
		}
		else
		{
			m_val.intVal = m_val.intVal;
		}

		m_type = rhs.m_type;

		return *this;
	}

	void VMData::Clear()
	{
		if (m_type.IsNative())
		{
			m_val.nativeVal->RemoveReference();
		}

		m_val.intVal = 0;
		m_type.Set(VMDATATYPE_INT);
	}
}
