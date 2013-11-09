// (c) 2004 DodoSoft Inc.
#if !defined(DSR_FUNCTION_H_)
#define DSR_FUNCTION_H_

#include "DSRPlatform.h"
#include "DSRDataType.h"
#include "DSRVMInstruction.h"
#include "DSRVMData.h"

namespace dsr
{
	class ScriptInstance;
	class VMData;

	//------------------------------------------------------------------------------------
	class FunctionDefinition
	{
		DSR_NOCOPY(FunctionDefinition)
	public:
		DSR_NEWDELETE(FunctionDefinition)

		const char* GetName() const { return m_name.c_str(); }
		uint32 GetNumArgs() const { return m_parameters.size(); }
		VMDataType GetArgVMDataType(uint32 idx) const { return m_parameters[idx]; }
		VMDataType GetReturnVMDataType() const { return m_returnType; }

	private:
		String m_name;
		VMDataType m_returnType;
		VMDataTypeArray m_parameters;
	};

	//------------------------------------------------------------------------------------
	class FunctionImplementation
	{
		DSR_NOCOPY(FunctionImplementation)
	public:
		DSR_NEWDELETE(FunctionImplementation)

		virtual ~FunctionImplementation() {}
		virtual void Call(ScriptInstance* pInstance, VMDataArray& args, VMData* retVal) const = 0;
		virtual bool IsNative() const = 0;
		const ScriptClass* GetScriptClassPtr() const { return m_scriptClass; }
		const FunctionDefinition* GetFunctionDefinitionPtr() const { return m_funcDef; }

	private:
		const ScriptClass* m_scriptClass;
		const FunctionDefinition* m_funcDef;
	};

	//------------------------------------------------------------------------------------
	class NativeFunctionImplementation : public FunctionImplementation
	{
		DSR_NOCOPY(NativeFunctionImplementation)
	public:
		DSR_NEWDELETE(NativeFunctionImplementation)
		typedef void NativeScriptFunction(ScriptInstance* pInst, VMDataArray& args, VMData* retVal);

		virtual void Call(ScriptInstance* pInstance, VMDataArray& args, VMData* retVal) const;
		virtual bool IsNative() const { return true; }
		void SetNativeScriptFunction(NativeScriptFunction* pFnc)
		{
			DSR_ASSERT(pFnc);
			DSR_ASSERT(!m_nativeFnc);
			m_nativeFnc = pFnc;
		}

	private:
		NativeScriptFunction* m_nativeFnc;
	};

	//------------------------------------------------------------------------------------
	class ScriptedFunctionImplementation : public FunctionImplementation
	{
		DSR_NOCOPY(ScriptedFunctionImplementation)
	public:
		DSR_NEWDELETE(ScriptedFunctionImplementation)

		virtual void Call(ScriptInstance* pInstance, VMDataArray& args, VMData* retVal) const;
		virtual bool IsNative() const { return false; }

	private:
		const char* GetNewClassName(uint32 idx) const;
		uint32 GetNumNewClassNames() const;

	private:
		VMCodeBlock m_vmcode;
		uint32 m_maxStackSize;
		VMDataTypeArray m_locals;
	};
}

#endif