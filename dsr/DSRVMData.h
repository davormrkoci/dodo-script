// (c) 2004 DodoSoft Inc.
#if !defined(DSR_VMDATA_H_)
#define DSR_VMDATA_H_

#include "DSRPlatform.h"
#include "DSRArray.h"
#include "DSRVMDataVal.h"
#include "DSRVMDataType.h"

namespace dsr
{
	class VMData
	{
	public:
		DSR_NEWDELETE(VMData)

		VMData();
		explicit VMData(float val);
		explicit VMData(int32 val);
		explicit VMData(bool val);
		VMData(ScriptInstanceHandle* pInst, VMDataType dataType);
		~VMData();

		VMData(const VMData& rhs);
		VMData& operator=(const VMData& rhs);

		void Clear();

		VMDataType GetVMDataType() const { return m_type; }
		void Set(float val);
		void Set(int32 val);
		void Set(bool val);
		void Set(ScriptInstanceHandle* pInst, VMDataType dataType);
		int32 GetInt() const;
		float GetFloat() const;
		bool GetBool() const;
		ScriptInstanceHandle* GetScriptInstanceHandlePtr() const { DSR_ASSERT(m_type.IsNative()); return m_val.nativeVal; }

	private:
		VMDataVal m_val;
		VMDataType m_type;
	};

	typedef Array<VMData> VMDataArray;
}

#endif