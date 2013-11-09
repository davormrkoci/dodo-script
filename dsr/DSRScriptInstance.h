#if !defined(DSR_SCRIPTINSTANCE_H_)
#define DSR_SCRIPTINSTANCE_H_

#include "DSRPlatform.h"
#include "DSRHandleTypedefs.h"
#include "DSRVMData.h"
#include "DSRScriptClass.h"

namespace dsr
{
	class VMData;
	class FunctionImplementation;

	class ScriptInstance
	{
		DSR_NOCOPY(ScriptInstance)
	public:
		DSR_NEWDELETE(ScriptInstance)

		explicit ScriptInstance(const ScriptClass* pClass);
		virtual ~ScriptInstance();

		void CallFunction(uint32 fnIdx, VMDataArray& args, VMData* retVal)
		{
			DSR_ASSERT(m_scriptClass);
			DSR_ASSERT(fnIdx < m_scriptClass->GetNumFunctions());
			m_scriptClass->GetFunctionImplementationPtr(fnIdx)->Call(this, args, retVal);
		}

		void CallConstructor(uint32 fnIdx, VMDataArray& args, VMData* retVal)
		{
			DSR_ASSERT(m_scriptClass);
			DSR_ASSERT(fnIdx < m_scriptClass->GetNumConstructors());
			m_scriptClass->GetConstructorImplementationPtr(fnIdx)->Call(this, args, retVal);
		}

		void ScriptInstance::CallFunction(const FunctionImplementation* pImpl, VMDataArray& args, VMData* retVal)
		{
			DSR_ASSERT(m_scriptClass);
			DSR_ASSERT(pImpl);
			DSR_ASSERT(m_scriptClass->IsA(pImpl->GetScriptClassPtr()));
			pImpl->Call(this, args, retVal);
		}

		const ScriptClass* GetScriptClassPtr() const { return m_scriptClass; }
		int32* GetInstanceData() { return m_instanceData; }
		const int32* GetInstanceData() const { return m_instanceData; }
		ScriptInstanceHandle* GetHandlePtr() { return m_handleOwner.GetHandlePtr(); }

	private:
		ScriptInstance();	//not implemented

	protected:
		const ScriptClass* m_scriptClass;
		int32* m_instanceData;
		HandleOwner<ScriptInstance> m_handleOwner;
	};
}

#endif