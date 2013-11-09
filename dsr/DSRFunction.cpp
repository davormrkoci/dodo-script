
#include "DSRFunction.h"

#include "DSRHandleTypedefs.h"
#include "DSRVMData.h"
#include "DSRScriptInstance.h"
#include "DSRScriptClass.h"
#include "DSRScriptManager.h"

namespace dsr
{
	void NativeFunctionImplementation::Call(ScriptInstance* pInstance, VMDataArray& args, VMData* retVal) const
	{
		DSR_ASSERT(m_nativeFnc);
		m_nativeFnc(pInstance, args, retVal);
	}

	//-------------------------------------------------------------------------
	DSR_INLINE VMInstruction ExtractVMInstruction(VMBytecode code)
	{
		return (code >> 24);
	}

	DSR_INLINE uint32 ExtractUnsignedValue(VMBytecode code)
	{
		return code & 0x00FFFFFF;
	}

	DSR_INLINE ScriptInstance* GetScriptInstancePtr(const VMData& data)
	{
		ScriptInstanceHandle* pH = data.GetScriptInstanceHandlePtr();
		DSR_ASSERT(pH);
		return pH->get();
	}

	DSR_INLINE void Pop(VMData*& pStack)
	{
		pStack->Clear();
		--pStack;
	}

	DSR_INLINE void Pop(VMData*& pStack, uint32 numPops)
	{
		for (uint32 i=0; i<numPops; ++i)
		{
			Pop(pStack);
		}
	}

	DSR_INLINE void Push(VMData*& pStack, const VMData& data)
	{
		++pStack;
		*pStack = data;
	}

	void ScriptedFunctionImplementation::Call(ScriptInstance* pInstance, VMDataArray& args, VMData* retVal) const
	{
		DSR_ASSERT(pInstance);
		DSR_ASSERT(GetFunctionDefinitionPtr()->GetNumArgs() == args.size());
		DSR_ASSERT(retVal);

		uint32 pc = 0;
		DSR_ASSERT(pc >= 0 && pc < m_vmcode.size());

		//create & initialize local data
		VMDataArray locals;
		locals.resize(m_locals.size());
		for (uint32 i=0; i<m_locals.size(); ++i)
		{
			switch (m_locals[i].GetVMDataTypeEnum())
			{
			case VMDATATYPE_NATIVE:
				locals[i].Set(new ScriptInstanceHandle(0), m_locals[i]);
				break;
			case VMDATATYPE_FLOAT:
				locals[i].Set(0.0f);
				break;
			case VMDATATYPE_INT:
				locals[i].Set(0);
				break;
			case VMDATATYPE_BOOL:
				locals[i].Set(false);
				break;
			default:
				DSR_ASSERT(false);
				break;
			}
		}

		//create stack
		VMDataArray stack;
		stack.resize(m_maxStackSize);

		//get data stack ptr
		VMData* pDataStack = 0;
		if (!stack.empty())
			pDataStack = &stack[0];

		//get instance's script type
		const ScriptClass* pExecutingST = pInstance->GetScriptClassPtr();

		bool done = false;
		while (!done)
		{
			VMBytecode curCode = m_vmcode[pc];

			switch (ExtractVMInstruction(curCode))
			{
			case VMI_NOP:
				break;

			case VMI_CALLC_PUSHED_G:
				{
					++pc;
					DSR_ASSERT(pc >= 0 && pc < m_vmcode.size());

					//get con idx
					const uint32 fnIdx = m_vmcode[pc];

					//pop value of the stack
					VMData val = *pDataStack;
					Pop(pDataStack);

					//get instance
					ScriptInstance* pPushedI = GetScriptInstancePtr(val);
					const ScriptClass* pPushedT = val.GetVMDataType().GetScriptClassPtr();
					DSR_ASSERT(pPushedT);
					DSR_ASSERT(pPushedI && pPushedI->GetScriptClassPtr()->IsA(pPushedT));

					//get num args
					DSR_ASSERT(fnIdx < pPushedT->GetNumConstructors());
					const uint32 numArgs = pPushedT->GetConstructorDefinitionPtr(fnIdx)->GetNumArgs();

					//get args
					VMDataArray args;
					args.resize(numArgs);
					VMData* pArgs = pDataStack - numArgs + 1;
					DSR_ASSERT(!stack.empty() && pArgs >= &stack[0]);
					for (uint32 i=0; i<numArgs; ++i)
					{
						args[i] = *pArgs;
						++pArgs;
					}

					//set return value
					VMData retVal;

					//call function
					pPushedI->CallConstructor(fnIdx, args, &retVal);

					//pop parameters off the stack
					Pop(pDataStack, numArgs);

					//push return value on the stack
					Push(pDataStack, retVal);
				}
				break;

			case VMI_CALLC_SELF_SUPER:
				{
					++pc;
					DSR_ASSERT(pc >= 0 && pc < m_vmcode.size());

					//get super script type
					ScriptClass* pSuperScriptClass = GetScriptClassPtr()->GetSuperPtr();
					DSR_ASSERT(pSuperScriptClass);

					//get fn idx
					const uint32 fnIdx = m_vmcode[pc];
					DSR_ASSERT(fnIdx < pSuperScriptClass->GetNumConstructors());

					//get function
					const FunctionDefinition* pFncDef = pSuperScriptClass->GetConstructorDefinitionPtr(fnIdx);
					const FunctionImplementation* pFncImp = pSuperScriptClass->GetConstructorImplementationPtr(fnIdx);
					DSR_ASSERT(pFncDef);
					DSR_ASSERT(pFncImp);

					//get num args
					const uint32 numArgs = pFncDef->GetNumArgs();

					//get args
					VMDataArray args;
					args.resize(numArgs);
					VMData* pArgs = pDataStack - numArgs + 1;
					DSR_ASSERT(!stack.empty() && pArgs >= &stack[0]);
					for (uint32 i=0; i<numArgs; ++i)
					{
						args[i] = *pArgs;
						++pArgs;
					}

					//set return value
					VMData retVal;

					//call function
					pInstance->CallFunction(pFncImp, args, &retVal);

					//pop parameters off the stack
					Pop(pDataStack, numArgs);

					//push return value on the stack
					Push(pDataStack, retVal);
				}
				break;

			case VMI_CALLF_PUSHED_G:
				{
					++pc;
					DSR_ASSERT(pc >= 0 && pc < m_vmcode.size());

					//get fn idx
					const uint32 fnIdx = m_vmcode[pc];

					//pop value of the stack
					VMData val = *pDataStack;
					Pop(pDataStack);

					//get instance
					ScriptInstance* pPushedI = GetScriptInstancePtr(val);
					const ScriptClass* pPushedT = val.GetVMDataType().GetScriptClassPtr();
					DSR_ASSERT(pPushedT);
					DSR_ASSERT(!pPushedI || pPushedI->GetScriptClassPtr()->IsA(pPushedT));

					//get num args
					DSR_ASSERT(fnIdx < pPushedT->GetNumFunctions());
					const uint32 numArgs = pPushedT->GetFunctionDefinitionPtr(fnIdx)->GetNumArgs();

					//get args
					VMDataArray args;
					args.resize(numArgs);
					VMData* pArgs = pDataStack - numArgs + 1;
					DSR_ASSERT(!stack.empty() && pArgs >= &stack[0]);
					for (uint32 i=0; i<numArgs; ++i)
					{
						args[i] = *pArgs;
						++pArgs;
					}

					//set return value
					VMData retVal;

					//call function
					if (pPushedI)
					{
						//get function
						pPushedI->CallFunction(fnIdx, args, &retVal);
					}
					else
					{
						//script instance is 0, just fill in default 0 values
						if (pPushedT->GetFunctionDefinitionPtr(fnIdx)->GetReturnVMDataType().IsNative())
						{
							retVal.Set(new ScriptInstanceHandle(0), pPushedT->GetFunctionDefinitionPtr(fnIdx)->GetReturnVMDataType());
						}
						else
						{
							retVal.Set(0);
						}
					}

					//pop parameters off the stack
					Pop(pDataStack, numArgs);

					//push return value on the stack
					Push(pDataStack, retVal);
				}
				break;

			case VMI_CALLF_SELF_G:
				{
					++pc;
					DSR_ASSERT(pc >= 0 && pc < m_vmcode.size());

					//get fn idx
					const uint32 fnIdx = m_vmcode[pc];
					DSR_ASSERT(fnIdx < pExecutingST->GetNumFunctions());

					//get function
					const FunctionDefinition* pFncDef = pExecutingST->GetFunctionDefinitionPtr(fnIdx);
					DSR_ASSERT(pFncDef);

					//get num args
					const uint32 numArgs = pFncDef->GetNumArgs();

					//get a pointer to the first parameter
					//get args
					VMDataArray args;
					args.resize(numArgs);
					VMData* pArgs = pDataStack - numArgs + 1;
					DSR_ASSERT(!stack.empty() && pArgs >= &stack[0]);
					for (uint32 i=0; i<numArgs; ++i)
					{
						args[i] = *pArgs;
						++pArgs;
					}

					//set return value
					VMData retVal;

					//call function
					pInstance->CallFunction(fnIdx, args, &retVal);

					//pop parameters off the stack
					Pop(pDataStack, numArgs);

					//push return value on the stack
					Push(pDataStack, retVal);
				}
				break;

			case VMI_CALLF_SUPER_G:
				{
					++pc;
					DSR_ASSERT(pc >= 0 && pc < m_vmcode.size());

					//get super script type
					ScriptClass* pSuperScriptClass = GetScriptClassPtr()->GetSuperPtr();
					DSR_ASSERT(pSuperScriptClass);

					//get fn idx
					const uint32 fnIdx = m_vmcode[pc];
					DSR_ASSERT(fnIdx < pSuperScriptClass->GetNumFunctions());

					//get function
					const FunctionDefinition* pFncDef = pSuperScriptClass->GetFunctionDefinitionPtr(fnIdx);
					const FunctionImplementation* pFncImp = pSuperScriptClass->GetFunctionImplementationPtr(fnIdx);
					DSR_ASSERT(pFncDef);
					DSR_ASSERT(pFncImp);

					//get num args
					const uint32 numArgs = pFncDef->GetNumArgs();

					//get args
					VMDataArray args;
					args.resize(numArgs);
					VMData* pArgs = pDataStack - numArgs + 1;
					DSR_ASSERT(!stack.empty() && pArgs >= &stack[0]);
					for (uint32 i=0; i<numArgs; ++i)
					{
						args[i] = *pArgs;
						++pArgs;
					}

					//set return value
					VMData retVal;

					//call function
					pInstance->CallFunction(pFncImp, args, &retVal);

					//pop parameters off the stack
					Pop(pDataStack, numArgs);

					//push return value on the stack
					Push(pDataStack, retVal);
				}
				break;

			case VMI_NEW:
				{
					uint32 classNameIdx = ExtractUnsignedValue(curCode);
					DSR_ASSERT(classNameIdx < GetNumNewClassNames());
					const ScriptClass* pClass = ScriptManagerPtr()->GetScriptClassPtr(GetNewClassName(classNameIdx));
					DSR_ASSERT(pClass);
					ScriptInstance* pInst = pClass->CreateInstance();
					DSR_ASSERT(pInst);

					VMDataType dataType(pClass);
					VMData data(pInst->GetHandlePtr(), dataType);

					//push return value on the stack
					Push(pDataStack, data);
				}
				break;

			case VMI_RET:
				{
					//get return value from the top of the stack
					*retVal = *pDataStack;

					//pop stack
					Pop(pDataStack);

					//we are done
					done = true;
				}
				break;

			case VMI_JMP:
				{
					pc = ExtractUnsignedValue(curCode) - 1;	//-1 beacuse we increment PC in the end
				}
				break;

			case VMI_JZ:
				{
					const bool jmp = pDataStack->GetInt() == 0;
					Pop(pDataStack);
					if (jmp)
						pc = ExtractUnsignedValue(curCode) - 1;	//-1 beacuse we increment PC in the end
				}
				break;

			case VMI_STORESF:
				{
					const float val = pDataStack->GetFloat();
					Pop(pDataStack);
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(dataOffset < pExecutingST->GetNumData());
					DSR_ASSERT(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_FLOAT);
					pInstance->GetInstanceData()[dataOffset] = *((int32*) &val);
				}
				break;

			case VMI_STORESI:
				{
					const int32 val = pDataStack->GetInt();
					Pop(pDataStack);
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(dataOffset < pExecutingST->GetNumData());
					DSR_ASSERT(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_INT);
					pInstance->GetInstanceData()[dataOffset] = *((int32*) &val);
				}
				break;

			case VMI_STORESB:
				{
					const int32 val = pDataStack->GetBool() ? 1 : 0;
					Pop(pDataStack);
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(dataOffset < pExecutingST->GetNumData());
					DSR_ASSERT(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_BOOL);
					pInstance->GetInstanceData()[dataOffset] = *((int32*) &val);
				}
				break;

			case VMI_STORESN:
				{
					ScriptInstanceHandle* pNTH = pDataStack->GetScriptInstanceHandlePtr();
					DSR_ASSERT(pNTH);

					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(dataOffset < pExecutingST->GetNumData());
					DSR_ASSERT(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().IsNative());
					DSR_ASSERT(!pNTH->get() || pNTH->get()->GetScriptClassPtr()->IsA(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().GetScriptClassPtr()));

					ScriptInstanceHandle** pData = ((ScriptInstanceHandle**)&(pInstance->GetInstanceData()[dataOffset]));
					DSR_ASSERT(pData);
					(*pData)->RemoveReference();
					*pData = pNTH;
					(*pData)->AddReference();

					Pop(pDataStack);
				}
				break;

			case VMI_STORELF:
			case VMI_STORELI:
			case VMI_STORELB:
			case VMI_STORELN:
				{
					VMData val = *pDataStack;
					const uint32 dataOffset = ExtractUnsignedValue(curCode);

					DSR_ASSERT(locals[dataOffset].GetVMDataType().GetVMDataTypeEnum()
						== val.GetVMDataType().GetVMDataTypeEnum());
					DSR_ASSERT(!val.GetVMDataType().IsNative()
						|| val.GetVMDataType().GetScriptClassPtr()->IsA(locals[dataOffset].GetVMDataType().GetScriptClassPtr()));

					locals[dataOffset] = val;
					Pop(pDataStack);
				}
				break;

			case VMI_STOREPF:
			case VMI_STOREPI:
			case VMI_STOREPB:
			case VMI_STOREPN:
				{
					VMData val = *pDataStack;
					const uint32 dataOffset = ExtractUnsignedValue(curCode);

					DSR_ASSERT(args[dataOffset].GetVMDataType().GetVMDataTypeEnum() == val.GetVMDataType().GetVMDataTypeEnum());
					DSR_ASSERT(!val.GetVMDataType().IsNative()
						|| val.GetVMDataType().GetScriptClassPtr()->IsA(args[dataOffset].GetVMDataType().GetScriptClassPtr()));

					args[dataOffset] = val;
					Pop(pDataStack);
				}
				break;

			case VMI_FETCHSF:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(dataOffset < pExecutingST->GetNumData());
					DSR_ASSERT(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_FLOAT);

					VMData vmData(*((float*)&(pInstance->GetInstanceData()[dataOffset])));

					Push(pDataStack, vmData);
				}
				break;

			case VMI_FETCHSI:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(dataOffset < pExecutingST->GetNumData());
					DSR_ASSERT(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_INT);

					VMData vmData(pInstance->GetInstanceData()[dataOffset]);

					Push(pDataStack, vmData);
				}
				break;

			case VMI_FETCHSB:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(dataOffset < pExecutingST->GetNumData());
					DSR_ASSERT(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_BOOL);

					VMData vmData(pInstance->GetInstanceData()[dataOffset] != 0);

					Push(pDataStack, vmData);
				}
				break;

			case VMI_FETCHSN:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(dataOffset < pExecutingST->GetNumData());
					DSR_ASSERT(pExecutingST->GetDataTypePtr(dataOffset)->GetVMDataType().IsNative());

					ScriptInstanceHandle* pSrc = *((ScriptInstanceHandle**)&(pInstance->GetInstanceData()[dataOffset]));
					VMData vmData(pSrc, pInstance->GetScriptClassPtr()->GetDataTypePtr(dataOffset)->GetVMDataType());

					Push(pDataStack, vmData);
				}
				break;

			case VMI_FETCHLF:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(locals[dataOffset].GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_FLOAT);
					Push(pDataStack, locals[dataOffset]);
				}
				break;

			case VMI_FETCHLI:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(locals[dataOffset].GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_INT);
					Push(pDataStack, locals[dataOffset]);
				}
				break;

			case VMI_FETCHLB:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(locals[dataOffset].GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_BOOL);
					Push(pDataStack, locals[dataOffset]);
				}
				break;

			case VMI_FETCHLN:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(locals[dataOffset].GetVMDataType().IsNative());
					Push(pDataStack, locals[dataOffset]);
				}
				break;

			case VMI_FETCHPF:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(args[dataOffset].GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_FLOAT);
					Push(pDataStack, args[dataOffset]);
				}
				break;

			case VMI_FETCHPI:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(args[dataOffset].GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_INT);
					Push(pDataStack, args[dataOffset]);
				}
				break;

			case VMI_FETCHPB:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(args[dataOffset].GetVMDataType().GetVMDataTypeEnum() == VMDATATYPE_BOOL);
					Push(pDataStack, args[dataOffset]);
				}
				break;

			case VMI_FETCHPN:
				{
					const uint32 dataOffset = ExtractUnsignedValue(curCode);
					DSR_ASSERT(args[dataOffset].GetVMDataType().IsNative());
					Push(pDataStack, args[dataOffset]);
				}
				break;

			case VMI_PUSHF:
				{
					++pc;
					Push(pDataStack, VMData(*((float*)&(m_vmcode[pc]))));
				}
				break;

			case VMI_PUSHI:
				{
					++pc;
					Push(pDataStack, VMData(*((int32*)&(m_vmcode[pc]))));
				}
				break;

			case VMI_PUSHB:
				{
					uint32 val = ExtractUnsignedValue(curCode);
					Push(pDataStack, VMData(val != 0));
				}
				break;			

			case VMI_POP:
				{
					Pop(pDataStack);
				}
				break;

			case VMI_NEGF:
				{
					pDataStack->Set(-pDataStack->GetFloat());
				}
				break;

			case VMI_NEGI:
				{
					pDataStack->Set(-pDataStack->GetInt());
				}
				break;

			case VMI_NOT:
				{
					pDataStack->Set(!pDataStack->GetBool());
				}
				break;

			case VMI_DIVII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const int32 res = val1 / val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_DIVFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const float res = val1 / val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_DIVFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const float res = val1 / ((float)val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_DIVIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const float res = ((float)val1) / val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_MULII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const int32 res = val1 * val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_MULFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const float res = val1 * val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_MULFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const float res = val1 * ((float) val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_MULIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const float res = ((float)val1) * val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_SUBII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const int32 res = val1 - val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_SUBFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const float res = val1 - val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_SUBFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const float res = val1 - ((float)val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_SUBIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const float res = ((float)val1) - val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_ADDII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const int32 res = val1 + val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_ADDFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const float res = val1 + val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_ADDFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const float res = val1 + ((float)val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_ADDIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const float res = ((float)val1) + val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_MOD:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const int32 res = val1 % val2;
					pDataStack->Set(res);
				}
				break;

			case VMI_EQII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (val1 == val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_EQFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 == val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_EQFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 == ((float)val2));
					pDataStack->Set(res);
				}
				break;

			case VMI_EQIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (((float)val1) == val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_EQBB:
				{
					const bool val2 = pDataStack->GetBool();
					Pop(pDataStack);
					const bool val1 = pDataStack->GetBool();
					const bool res = (val1 == val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_LTEQII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (val1 <= val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_LTEQFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 <= val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_LTEQFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 <= ((float)val2));
					pDataStack->Set(res);
				}
				break;

			case VMI_LTEQIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (((float)val1) <= val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_LTII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (val1 < val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_LTFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 < val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_LTFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 < ((float)val2));
					pDataStack->Set(res);
				}
				break;

			case VMI_LTIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (((float)val1) < val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_GTEQII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (val1 >= val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_GTEQFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 >= val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_GTEQFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 >= ((float)val2));
					pDataStack->Set(res);
				}
				break;

			case VMI_GTEQIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (((float)val1) >= val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_GTII:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (val1 > val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_GTFF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 > val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_GTFI:
				{
					const int32 val2 = pDataStack->GetInt();
					Pop(pDataStack);
					const float val1 = pDataStack->GetFloat();
					const bool res = (val1 > ((float)val2));
					pDataStack->Set(res);
				}
				break;

			case VMI_GTIF:
				{
					const float val2 = pDataStack->GetFloat();
					Pop(pDataStack);
					const int32 val1 = pDataStack->GetInt();
					const bool res = (((float)val1) > val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_AND:
				{
					const bool val2 = pDataStack->GetBool();
					Pop(pDataStack);
					const bool val1 = pDataStack->GetBool();
					const bool res = (val1 && val2);
					pDataStack->Set(res);
				}
				break;

			case VMI_OR:
				{
					const bool val2 = pDataStack->GetBool();
					Pop(pDataStack);
					const bool val1 = pDataStack->GetBool();
					const bool res = (val1 || val2);
					pDataStack->Set(res);
				}
				break;

			default:
				{
					//unimplemented instruction
					DSR_ASSERT(false);
					done = true;
				}
				break;
			};

			++pc;
		}

		DSR_ASSERT(pDataStack == 0 || pDataStack == &stack[0]);
	}
}