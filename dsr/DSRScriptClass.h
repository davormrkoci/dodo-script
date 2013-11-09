#if !defined(DSR_SCRIPTCLASS_H_)
#define DSR_SCRIPTCLASS_H_

#include "DSRPlatform.h"
#include "DSRArray.h"
#include "DSRFunction.h"
#include "DSRFunctionVTable.h"

namespace dsr
{
	class ScriptFactory;

	class ScriptClass
	{
		DSR_NOCOPY(ScriptClass)
	public:
		DSR_NEWDELETE(ScriptClass)

		typedef Array<FunctionImplementation*> FunctionImplementationPtrArray;
		typedef Array<FunctionDefinition> FunctionDefinitionArray;
		typedef Array<DataType> DataTypeArray;

		~ScriptClass();

		const char* GetName() const { return m_name.c_str(); }
		ScriptClass* GetSuperPtr() const { return m_super; }
		bool IsNative() const { return m_native; }
		int32 GetFunctionVTableIndex(const char* functionName) const;
		int32 GetDataTypeIndex(const char* dataName) const;
		const FunctionImplementation* GetFunctionImplementationPtr(uint32 fnIdx) const;
		const FunctionDefinition* GetFunctionDefinitionPtr(uint32 fnIdx) const;
		const DataType* GetDataTypePtr(uint32 dataIdx) const;
		uint32 GetNumFunctions() const { return m_functionVTable.GetNumFunctions(); }
		uint32 GetNumData() const
		{
			if (m_super)
				return m_data.size() + m_super->GetNumData();
			else
				return m_data.size();
		}
		uint32 GetNumConstructors() const { return m_constructorDefs.size(); }
		const FunctionImplementation* GetConstructorImplementationPtr(uint32 cnIdx) const
		{
			DSR_ASSERT(cnIdx >=0 && cnIdx < GetNumConstructors());
			return m_constructorImps[cnIdx];
		}
		const FunctionDefinition* GetConstructorDefinitionPtr(uint32 cnIdx) const
		{
			DSR_ASSERT(cnIdx >=0 && cnIdx < GetNumConstructors());
			return &m_constructorDefs[cnIdx];
		}

		//used by factory only
		void SetNativeFunction(uint32 fncIdx, NativeFunctionImplementation::NativeScriptFunction* pFunc);
		void SetNativeConstructor(uint32 cnIdx, NativeFunctionImplementation::NativeScriptFunction* pFunc);

		//is a relationship
		bool IsA(const ScriptClass* pScriptType) const;
		const ScriptClass* GetClosestNativeClassPtr() const;

		ScriptInstance* CreateInstance() const;

	private:
		ScriptClass();

	private:
		String m_name;
		bool m_native;
		ScriptClass* m_super;
		FunctionImplementationPtrArray m_constructorImps;
		FunctionDefinitionArray m_constructorDefs;
		FunctionImplementationPtrArray m_funcImps;
		FunctionDefinitionArray m_funcDefs;
		FunctionVTable m_functionVTable;
		DataTypeArray m_data;
		ScriptFactory* m_pFactory;
	};
}

#endif