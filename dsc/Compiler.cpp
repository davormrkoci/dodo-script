#include <stdarg.h>
#include <algorithm>
#include "File.h"
#include "StringUtils.h"
#include "Parser.h"
#include "Compiler.h"

namespace dsc
{
	Compiler* Compiler::m_pInstance = 0;

	class FindByName
	{
	public:
		explicit FindByName(const char* name)
		{
			m_name = name;
		}

		template <class T>
		bool operator()(const T& data) const
		{
			return strcmp(data->GetName(), m_name.c_str()) == 0;
		}

	private:
		std::string m_name;
	};

	void Compiler::Create()
	{
		if (!m_pInstance)
			m_pInstance = new Compiler();
	}

	void Compiler::Destroy()
	{
		delete m_pInstance;
		m_pInstance = 0;
	}

	static const char* FORMAT(const char *fmt, ...)
	{
		static char str[1024];

		va_list ap;
		va_start(ap, fmt);
		_vsnprintf(str, 1024, fmt, ap);
		va_end(ap);

		return str;
	}

	std::string VMDataTypeToString(uint32 dataType)
	{
		std::string ret;
		switch (dataType)
		{
		case dsr::VMDATATYPE_BOOL:
			ret = "bool";
			break;
		case dsr::VMDATATYPE_INT:
			ret = "int";
			break;
		case dsr::VMDATATYPE_NATIVE:
			ret = "native";
			break;
		case dsr::VMDATATYPE_VOID:
			ret = "void";
			break;
		case dsr::VMDATATYPE_FLOAT:
			ret = "float";
			break;
		}

		return ret;
	}

	const FunctionDeclaration* GetFunctionDeclarationPtr(const ScriptClassDeclaration* pDecl, uint32 funcIdx, uint32 ctorIdx)
	{
		assert(funcIdx == -1 || ctorIdx == -1);
		assert(funcIdx != ctorIdx);

		if (funcIdx != -1)
			return pDecl->GetFunctionDeclarationPtr(funcIdx);
		else
			return pDecl->GetConstructorFunctionDeclarationPtr(ctorIdx);
	}

	dsr::VMBytecode BuildCode(int32 type, uint32 value = 0)
	{
		return (type << 24) | (value & 0x00FFFFFF);
	}

	template<class T>
	dsr::VMBytecode BuildData(T f)
	{
		assert(sizeof(f) == 4);
		return *((uint32*)(&f));
	}

	uint32 ExtractUnsignedValue(dsr::VMBytecode code)
	{
		return code & 0x00FFFFFF;
	}

	dsr::VMInstruction ExtractVMInstruction(dsr::VMBytecode code)
	{
		return (code >> 24);
	}

	uint32 GetDataType(const std::string& typeName)
	{
		if (strcmp(typeName.c_str(), "float") == 0)
			return dsr::VMDATATYPE_FLOAT;
		else if (strcmp(typeName.c_str(), "int") == 0)
			return dsr::VMDATATYPE_INT;
		else if (strcmp(typeName.c_str(), "bool") == 0)
			return dsr::VMDATATYPE_BOOL;
		else if (strcmp(typeName.c_str(), "void") == 0)
			return dsr::VMDATATYPE_VOID;
		else
			return dsr::VMDATATYPE_NATIVE;
	}

	bool IsValue(uint32 tokType)
	{
		switch (tokType)
		{
		case TOKEN_IDENTIFIER:
		case TOKEN_TRUE:
		case TOKEN_FALSE:
		case TOKEN_INTEGER_LITERAL:
		case TOKEN_FLOAT_LITERAL:
			return true;
			break;
		};

		return false;
	}

	bool IsUnaryOperation(uint32 tokType)
	{
		switch (tokType)
		{
		case TOKEN_UNARY_MINUS:
		case TOKEN_NOT:
			return true;
			break;
		};

		return false;
	}

	bool IsBinaryOperation(uint32 tokType)
	{
		switch (tokType)
		{
		case TOKEN_DIVIDE:
		case TOKEN_MULTIPLY:
		case TOKEN_MINUS:
		case TOKEN_PLUS:
		case TOKEN_MODULO:
		case TOKEN_EQUALS:
		case TOKEN_NOT_EQUALS:
		case TOKEN_LTEQ:
		case TOKEN_LT:
		case TOKEN_GTEQ:
		case TOKEN_GT:
		case TOKEN_AND:
		case TOKEN_OR:
			return true;
			break;
		};

		return false;
	}

	void LoadScriptFile(const char* fileName, std::string& data)
	{
		File file;
		if (!file.Open(fileName, File::READ_BINARY))
			throw CompilerException(FORMAT("Could not open file %s.", fileName));

		char* pData = new char[file.Size() + 1];
		file.Read(pData, file.Size());
		pData[file.Size()] = '\0';
		data = pData;
		delete[] pData;
	}

	std::string GenerateScriptFileName(const char* scriptName)
	{
		std::string fileName = scriptName;
		ReplaceChar(fileName, '.', '/');
		fileName += ".ds";
		return fileName;
	}

	//----------------------------------------------------------------------
	Compiler::Compiler()
	{
		Clear();
	}

	Compiler::~Compiler()
	{
		Clear();
	}

	void Compiler::Clear()
	{
		m_curStackSize = 0;
		m_maxStackSize = 0;
		m_curCode.clear();
		m_error = "";
		m_pDeclaration = 0;
		m_curFuncIdx = -1;
		m_curCtorIdx = -1;
		m_declarationList.clear();
		m_sources.clear();
		m_pCurFuncImpl = 0;
		m_curCompilePass = COMPILEPASS_UNDEF;
	}

	ScriptClassDeclarationCPtr Compiler::BuildScriptClassDeclarationPass1(const ScriptSource& scriptSource) const
	{
		ScriptClassDeclarationCPtr curDeclaration = new ScriptClassDeclaration();

		//add comments
		for (uint32 si = 0; si<scriptSource.GetNumComments(); ++si)
		{
			curDeclaration->AddComment(scriptSource.GetComment(si));
		}

		//set name
		curDeclaration->SetName(scriptSource.GetName());

		//set native
		curDeclaration->SetNative(scriptSource.IsNative());

		//set super
		curDeclaration->SetSuperClassName(scriptSource.GetSuper());

		//set data
		for (uint32 i=0; i<scriptSource.GetNumData(); ++i)
		{
			const DataSrc* pDataSrc = scriptSource.GetDataSrcPtr(i);
			assert(pDataSrc);
			DataDeclarationCPtr dataDeclaration = BuildDataDeclaration(*pDataSrc);
			curDeclaration->AddDataDeclaration(dataDeclaration);
		}

		//set function
		for (uint32 i=0; i<scriptSource.GetNumFunctions(); ++i)
		{
			const FunctionSrc* pFncSrc = scriptSource.GetFunctionSrcPtr(i);
			assert(pFncSrc);
			FunctionDeclarationCPtr fncDecl = BuildFunctionDeclaration(*pFncSrc);

			//add function definition to script type
			curDeclaration->AddFunctionDeclaration(fncDecl);
		}

		//set constructors
		for (uint32 i=0; i<scriptSource.GetNumConstructors(); ++i)
		{
			const FunctionSrc* pFncSrc = scriptSource.GetConstructorFunctionSrcPtr(i);
			assert(pFncSrc);
			FunctionDeclarationCPtr fncDecl = BuildFunctionDeclaration(*pFncSrc);

			//add function definition to script type
			curDeclaration->AddConstructorDeclaration(fncDecl);
		}

		return curDeclaration;
	}

	void Compiler::ClearCurCode()
	{
		m_curCode.clear();
		m_curStackSize = 0;
		m_maxStackSize = 0;
	}

	void Compiler::VisitBlock(const StBlock& stBlock)
	{
		for (uint32 i=0; i<stBlock.GetNumStatements(); ++i)
		{
			const StatementSrc* statement = stBlock.GetStatementSrcPtr(i);
			statement->Visit(*this);
		}
	}

	void Compiler::VisitWhile(const StWhile& stWhile)
	{
		assert(m_curStackSize == 0);

		const uint32 loopPos = (uint32) m_curCode.size();
		VisitExpressionAndCheckRetTypes(*(stWhile.GetConditionExpressionSrcPtr()), dsr::VMDATATYPE_BOOL, 0);

		m_curCode.push_back(BuildCode(dsr::VMI_INVALID));	//jz to be added later
		DecStackSize();
		const uint32 jzPos = (uint32) (m_curCode.size() - 1);

		stWhile.GetStatementSrcPtr()->Visit(*this);

		m_curCode.push_back(BuildCode(dsr::VMI_JMP, loopPos));
		assert(loopPos == ExtractUnsignedValue(m_curCode.back()));

		const uint32 endOfLoopPos = (uint32) m_curCode.size();

		//add missing jz (backpatching)
		m_curCode[jzPos] = BuildCode(dsr::VMI_JZ, endOfLoopPos);
		assert(endOfLoopPos == ExtractUnsignedValue(m_curCode[jzPos]));

		assert(m_curStackSize == 0);
	}

	void Compiler::VisitIf(const StIf& stIf)
	{
		assert(m_curStackSize == 0);

		VisitExpressionAndCheckRetTypes(*(stIf.GetConditionExpressionSrcPtr()), dsr::VMDATATYPE_BOOL, 0);

		m_curCode.push_back(BuildCode(dsr::VMI_INVALID));	//jz to be added later
		DecStackSize();
		uint32 jzPos = (uint32) (m_curCode.size() - 1);

		stIf.GetTrueStatementSrcPtr()->Visit(*this);

		uint32 falsePos = (uint32) m_curCode.size();
		if (stIf.GetFalseStatementSrcPtr())
		{
			stIf.GetFalseStatementSrcPtr()->Visit(*this);
		}

		//add missing jz (backpatching)
		m_curCode[jzPos] = BuildCode(dsr::VMI_JZ, falsePos);

		assert(m_curStackSize == 0);
	}

	void Compiler::AssignToVariable(const VarInfo& varInfo)
	{
		assert(m_pCurFuncImpl);

		//assign result to a variable
		if (varInfo.GetDataLoc() == Compiler::VarInfo::DATALOC_SCRIPT)
		{
			switch (varInfo.GetDataType())
			{
			case dsr::VMDATATYPE_FLOAT:
				m_curCode.push_back(BuildCode(dsr::VMI_STORESF, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_INT:
				m_curCode.push_back(BuildCode(dsr::VMI_STORESI, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_BOOL:
				m_curCode.push_back(BuildCode(dsr::VMI_STORESB, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_NATIVE:
				m_curCode.push_back(BuildCode(dsr::VMI_STORESN, varInfo.GetDataOffset()));
				break;

			default:
				assert(false);
				throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
				break;

			};
		}
		else if (varInfo.GetDataLoc() == Compiler::VarInfo::DATALOC_LOCAL)
		{
			switch (varInfo.GetDataType())
			{
			case dsr::VMDATATYPE_FLOAT:
				m_curCode.push_back(BuildCode(dsr::VMI_STORELF, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_INT:
				m_curCode.push_back(BuildCode(dsr::VMI_STORELI, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_BOOL:
				m_curCode.push_back(BuildCode(dsr::VMI_STORELB, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_NATIVE:
				m_curCode.push_back(BuildCode(dsr::VMI_STORELN, varInfo.GetDataOffset()));
				break;

			default:
				assert(false);
				throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
				break;

			};
		}
		else if (varInfo.GetDataLoc() == Compiler::VarInfo::DATALOC_PARAMETER)
		{
			switch (varInfo.GetDataType())
			{
			case dsr::VMDATATYPE_FLOAT:
				m_curCode.push_back(BuildCode(dsr::VMI_STOREPF, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_INT:
				m_curCode.push_back(BuildCode(dsr::VMI_STOREPI, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_BOOL:
				m_curCode.push_back(BuildCode(dsr::VMI_STOREPB, varInfo.GetDataOffset()));
				break;

			case dsr::VMDATATYPE_NATIVE:
				m_curCode.push_back(BuildCode(dsr::VMI_STOREPN, varInfo.GetDataOffset()));
				break;

			default:
				assert(false);
				throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
				break;
			};
		}

		DecStackSize();
	}

	void Compiler::VisitAssign(const StAssign& stAssign)
	{
		assert(m_curStackSize == 0);

		//find variable
		VarInfo varInfo;
		GetVarInfo(stAssign.GetVariableName(), varInfo);

		//evaluate expression
		VisitExpressionAndCheckRetTypes(*(stAssign.GetExpressionSrcPtr()), varInfo.GetDataType(), varInfo.GetDataNativeType());

		AssignToVariable(varInfo);

		assert(m_curStackSize == 0);
	}

	void Compiler::VisitReturn(const StReturn& stReturn)
	{
		assert(m_curStackSize == 0);

		//get return type
		uint32 retType = GetFunctionDeclarationPtr(m_pDeclaration, m_curFuncIdx, m_curCtorIdx)->GetReturnType();
		std::string nativeRetType = GetFunctionDeclarationPtr(m_pDeclaration, m_curFuncIdx, m_curCtorIdx)->GetNativeReturnType();

		if (retType == dsr::VMDATATYPE_VOID)
		{
			if (stReturn.GetReturnValueExpressionSrcPtr())
			{
				assert(false);
				throw CompilerException(FORMAT("Cannot return a value from a void function. Class %s, line %u.", m_pDeclaration->GetName(), stReturn.GetLine()));
			}

			//push bogus data on the stack.  this will get popped automatically.
			m_curCode.push_back(BuildCode(dsr::VMI_PUSHB));
			IncStackSize();
		}
		else
		{
			//evaluate return expression
			if (!(stReturn.GetReturnValueExpressionSrcPtr()))
				throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));

			VisitExpressionAndCheckRetTypes(*(stReturn.GetReturnValueExpressionSrcPtr()), retType, nativeRetType.c_str());
		}

		m_curCode.push_back(BuildCode(dsr::VMI_RET));
		DecStackSize();

		assert(m_curStackSize == 0);
	}

	void Compiler::VisitFunctionCall(const StFunctionCall& fncCall)
	{
		assert(m_curStackSize == 0);

		assert(fncCall.GetFunctionCallSrcPtr());
		uint32 retValType = dsr::VMDATATYPE_MAX;
		std::string nt;

		//visit function call
		VisitFunctionCall(*(fncCall.GetFunctionCallSrcPtr()), retValType, nt, "");

		//pop the return value
		m_curCode.push_back(BuildCode(dsr::VMI_POP));
		DecStackSize();

		assert(m_curStackSize == 0);
	}

	void Compiler::VisitFunctionCall(const FunctionCallSrc& fncCallSrc, uint32& retValType, std::string& nativeRetType, const char* pushedType)
	{
		//init return values
		retValType = dsr::VMDATATYPE_MAX;
		nativeRetType = "";

		if (fncCallSrc.IsNew())
		{
			//call new
			uint32 nameIdx = m_pCurFuncImpl->AddNewClassName(fncCallSrc.GetName());
			m_curCode.push_back(BuildCode(dsr::VMI_NEW, nameIdx));
			pushedType = fncCallSrc.GetName();
		}

		const bool isPushedCall = strcmp(pushedType, "") != 0;
		std::string type = m_pDeclaration->GetName();
		if (isPushedCall)
		{
			if (fncCallSrc.IsSuper() || fncCallSrc.IsVarCall() || fncCallSrc.IsBaseConstructor())
				throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));

			type = pushedType;
		}

		if (fncCallSrc.IsVarCall())
		{
			if (fncCallSrc.IsNew() || fncCallSrc.IsSuper() || isPushedCall || fncCallSrc.IsBaseConstructor())
				throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));

			VarInfo varInfo;
			GetVarInfo(fncCallSrc.GetVarName(), varInfo);

			//check if variable is native
			type = varInfo.GetDataNativeType();
		}

		const ScriptClassDeclaration* pDeclaration = GetScriptClassDeclarationPtr(type.c_str());

		//evaluate parameter expressions push them onto the stack
		ScriptClassDeclaration::DataDeclarationArray expTypes;
		expTypes.resize(fncCallSrc.GetNumParameters());
		for (uint32 i=0; i<fncCallSrc.GetNumParameters(); ++i)
		{
			uint32 retType;
			std::string retNatType;
			const ExpressionSrc* pExpr = fncCallSrc.GetParameterExpressionSrcPtr(i);
			assert(pExpr);
			VisitExpression(*pExpr, retType, retNatType);
			expTypes[i].SetType(retType);
			expTypes[i].SetNativeType(retNatType.c_str());
		}

		//get function decl
		const FunctionDeclaration* pFuncDecl = 0;
		int32 fnIdx = -1;
		if (fncCallSrc.IsNew())
		{
			fnIdx = pDeclaration->GetConstructorIndex(expTypes);
			if (fnIdx == -1)
			{
				throw CompilerException(FORMAT("No appropriate constructor available.  Class %s, line %u.",
					m_pDeclaration->GetName(), fncCallSrc.GetLine()));
			}
			pFuncDecl = pDeclaration->GetConstructorFunctionDeclarationPtr(fnIdx);
		}
		else if (fncCallSrc.IsBaseConstructor())
		{
			fnIdx = GetScriptClassDeclarationPtr(pDeclaration->GetName())->GetConstructorIndex(expTypes);
			if (fnIdx == -1)
			{
				throw CompilerException(FORMAT("No appropriate constructor available.  Class %s, line %u.",
					m_pDeclaration->GetName(), fncCallSrc.GetLine()));
			}
			pFuncDecl = GetScriptClassDeclarationPtr(pDeclaration->GetName())->GetConstructorFunctionDeclarationPtr(fnIdx);
		}
		else
		{
			fnIdx = pDeclaration->GetFunctionIndex(fncCallSrc.GetName());
			if (fnIdx == -1)
			{
				throw CompilerException(FORMAT("Unknown function \"%s\"called.  Class %s, line %u.",
					fncCallSrc.GetName(), m_pDeclaration->GetName(), fncCallSrc.GetLine()));
			}
			pFuncDecl = pDeclaration->GetFunctionDeclarationPtr(fnIdx);
		}

		if (pFuncDecl)
		{
			//check number of parameters
			if (fncCallSrc.GetNumParameters() != pFuncDecl->GetNumParameters())
			{
				throw CompilerException(FORMAT("Number of parameters for function \"%s\" does not match.  Class %s, line %u.",
					fncCallSrc.GetName(), m_pDeclaration->GetName(), fncCallSrc.GetLine()));
			}

			//check parameter types
			for (uint32 i=0; i<pFuncDecl->GetNumParameters(); ++i)
			{
				const DataDeclaration* pParam = pFuncDecl->GetParameterDataDeclarationPtr(i);
				assert(pParam);

				if (pParam->GetType() != expTypes[i].GetType())
				{
					throw CompilerException(FORMAT("Parameter \"%s\" for function \"%s\" does not match function declaration.  Class %s, line %u.",
						pParam->GetName(), fncCallSrc.GetName(), m_pDeclaration->GetName(), fncCallSrc.GetLine()));
				}

				if (pParam->GetType() == dsr::VMDATATYPE_NATIVE)
				{
					//dangerous upcasting here
					//TODO: remove if no upcasting is needed
					if (!IsA(expTypes[i].GetNativeType(), pParam->GetNativeType())
						&& !IsA(pParam->GetNativeType(), expTypes[i].GetNativeType()))
					{
						throw CompilerException(FORMAT("Parameter \"%s\" for function \"%s\" does not match function declaration.  Class %s, line %u.",
							pParam->GetName(), fncCallSrc.GetName(), m_pDeclaration->GetName(), fncCallSrc.GetLine()));
					}
				}
			}

			//get rtype of return value
			retValType = pFuncDecl->GetReturnType();
			nativeRetType = pFuncDecl->GetNativeReturnType();
			if (retValType != dsr::VMDATATYPE_NATIVE)
			{
				if (fncCallSrc.GetNextFncCallSrcPtr())
				{
					assert(false);
					throw CompilerException(FORMAT("Can't call functions on atomic types.  Class %s, line %u.",
						m_pDeclaration->GetName(), fncCallSrc.GetLine()));
				}
			}

			//to call function, you must do the following
			//  - push parameters
			//  - call function
			//  - function needs to clean up the parameters
			//  - push return value onto the stack (even if ret type is void)
			//after the function call, the return value should be on top of the stack.

			//call function
			if (fncCallSrc.IsSuper())
			{
				assert(!fncCallSrc.IsVarCall() && !isPushedCall && !fncCallSrc.IsNew());
				const ScriptClassDeclaration* pSuperDeclaration = GetScriptClassDeclarationPtr(pDeclaration->GetSuperClassName());
				if ((uint32) fnIdx >= pSuperDeclaration->GetNumFunctions())
					throw CompilerException(FORMAT("Super type class \"%s\" does not have function %s.  Class %s, line %u.", pDeclaration->GetSuperClassName(), fncCallSrc.GetName(), m_pDeclaration->GetName(), fncCallSrc.GetLine()));
				m_curCode.push_back(BuildCode(dsr::VMI_CALLF_SUPER_G));
				m_curCode.push_back(BuildData(fnIdx));
			}
			else if (fncCallSrc.IsNew())
			{
				//call constructor on the native type that is on the stack
				m_curCode.push_back(BuildCode(dsr::VMI_CALLC_PUSHED_G));
				m_curCode.push_back(BuildData(fnIdx));
				DecStackSize();
			}
			else if (fncCallSrc.IsBaseConstructor())
			{
				//call base constructor on self
				m_curCode.push_back(BuildCode(dsr::VMI_CALLC_SELF_SUPER));
				m_curCode.push_back(BuildData(fnIdx));
			}
			else if (fncCallSrc.IsVarCall() || isPushedCall)
			{
				assert(!fncCallSrc.IsNew());

				//push the native type onto the stack
				if (fncCallSrc.IsVarCall())
				{
					uint32 tempType;
					std::string tempNT;
					Token tok(TOKEN_IDENTIFIER, fncCallSrc.GetVarName(), 0, 0);
					ExprPushValue(tok, tempType, tempNT);
				}

				//call function on the native type that is on the stack
				m_curCode.push_back(BuildCode(dsr::VMI_CALLF_PUSHED_G));
				m_curCode.push_back(BuildData(fnIdx));
				DecStackSize();
			}
			else
			{
				assert(!fncCallSrc.IsNew());
				m_curCode.push_back(BuildCode(dsr::VMI_CALLF_SELF_G));
				m_curCode.push_back(BuildData(fnIdx));
			}

			for (uint32 i=0; i<pFuncDecl->GetNumParameters(); ++i)
				DecStackSize();
			IncStackSize();

			//check next call
			if (fncCallSrc.GetNextFncCallSrcPtr())
			{
				uint32 rvt;
				std::string nt;
				VisitFunctionCall(*(fncCallSrc.GetNextFncCallSrcPtr()), rvt, nt, nativeRetType.c_str());
			}
		}
		else
		{
			throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
		}
	}

	void Compiler::ExprPushValue(const Token& tok, uint32& type, std::string& nativeType)
	{
		switch (tok.GetType())
		{
		case TOKEN_IDENTIFIER:
			{
				VarInfo varInfo;
				GetVarInfo(tok.GetSpelling(), varInfo);

				if (varInfo.GetDataLoc() == Compiler::VarInfo::DATALOC_PARAMETER)
				{
					switch (varInfo.GetDataType())
					{
					case dsr::VMDATATYPE_FLOAT:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHPF, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_INT:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHPI, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_BOOL:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHPB, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_NATIVE:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHPN, varInfo.GetDataOffset()));
						break;

					default:
						throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
						break;
					};

					type = varInfo.GetDataType();
					nativeType = varInfo.GetDataNativeType();
					IncStackSize();
					return;
				}
				else if (varInfo.GetDataLoc() == Compiler::VarInfo::DATALOC_LOCAL)
				{
					switch (varInfo.GetDataType())
					{
					case dsr::VMDATATYPE_FLOAT:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHLF, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_INT:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHLI, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_BOOL:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHLB, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_NATIVE:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHLN, varInfo.GetDataOffset()));
						break;

					default:
						throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
						break;
					};

					type = varInfo.GetDataType();
					nativeType = varInfo.GetDataNativeType();
					IncStackSize();
					return;
				}
				else if (varInfo.GetDataLoc() == Compiler::VarInfo::DATALOC_SCRIPT)
				{
					switch (varInfo.GetDataType())
					{
					case dsr::VMDATATYPE_FLOAT:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHSF, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_INT:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHSI, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_BOOL:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHSB, varInfo.GetDataOffset()));
						break;
					case dsr::VMDATATYPE_NATIVE:
						m_curCode.push_back(BuildCode(dsr::VMI_FETCHSN, varInfo.GetDataOffset()));
						break;

					default:
						throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
						break;
					};

					type = varInfo.GetDataType();
					nativeType = varInfo.GetDataNativeType();
					IncStackSize();
					return;
				}
				else
				{
					throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
				}
			}
			break;

		case TOKEN_TRUE:
			{
				m_curCode.push_back(BuildCode(dsr::VMI_PUSHB, 1));
				type = dsr::VMDATATYPE_BOOL;
				nativeType = "";
				IncStackSize();
				return;
			}
			break;

		case TOKEN_FALSE:
			{
				m_curCode.push_back(BuildCode(dsr::VMI_PUSHB, 0));
				type = dsr::VMDATATYPE_BOOL;
				nativeType = "";
				IncStackSize();
				return;
			}
			break;

		case TOKEN_INTEGER_LITERAL:
			{
				m_curCode.push_back(BuildCode(dsr::VMI_PUSHI));
				m_curCode.push_back(BuildData(atoi(tok.GetSpelling())));
				type = dsr::VMDATATYPE_INT;
				nativeType = "";
				IncStackSize();
				return;
			}
			break;

		case TOKEN_FLOAT_LITERAL:
			{
				m_curCode.push_back(BuildCode(dsr::VMI_PUSHF));
				m_curCode.push_back(BuildData((float) atof(tok.GetSpelling())));
				type = dsr::VMDATATYPE_FLOAT;
				nativeType = "";
				IncStackSize();
				return;
			}
			break;

		default:
			throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
			break;
		};

		throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
	}

	void Compiler::VisitExpressionAndCheckRetTypes(const ExpressionSrc& expr, uint32 returnType, const char* nativeReturnType)
	{
		uint32 ret;
		std::string nativeRet;

		VisitExpression(expr, ret, nativeRet);

		//check native
		if (returnType == dsr::VMDATATYPE_NATIVE && ret == returnType)
		{
			if (IsA(nativeRet.c_str(), nativeReturnType))
				return;

			//dangerous upcasting here
			//TODO: remove if no upcasting is needed
			if (IsA(nativeReturnType, nativeRet.c_str()))
				return;
		}
		else if (ret == returnType)
		{
			return;
		}

		std::string expectedReturnType;
		if (returnType == dsr::VMDATATYPE_NATIVE)
			expectedReturnType = nativeReturnType;
		else
			expectedReturnType = VMDataTypeToString(returnType);
		std::string expressionReturnType;
		if (ret == dsr::VMDATATYPE_NATIVE)
			expressionReturnType = nativeRet;
		else
			expressionReturnType = VMDataTypeToString(ret);
		throw CompilerException(FORMAT("Expecting return type %s, but expression evaluates to %s.  Class %s, line %u.",
			expectedReturnType.c_str(), expressionReturnType.c_str(),
			m_pDeclaration->GetName(), expr.GetLine()));
	}

	void Compiler::VisitExpression(const ExpressionSrc& expr, uint32& retType, std::string& nativeRetType)
	{
		//init ret vals
		retType = dsr::VMDATATYPE_MAX;
		nativeRetType = "";

		if (expr.GetNumExpressionMembers() == 0)
			throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));

		std::list<uint32> typeStack;
		std::list<std::string> nativeTypeStack;
		for (uint32 i=0; i<expr.GetNumExpressionMembers(); ++i)
		{
			const ExpressionSrc::ExpressionMember& member = *expr.GetExpressionMemberPtr(i);
			if (member.GetFunctionCallSrcPtr())
			{
				uint32 retValType = dsr::VMDATATYPE_MAX;
				std::string nativeRetValType = "";
				VisitFunctionCall(*(member.GetFunctionCallSrcPtr()), retValType, nativeRetValType, "");
				typeStack.push_back(retValType);
				nativeTypeStack.push_back(nativeRetValType);
			}
			else
			{
				if (IsValue(member.GetToken().GetType()))
				{
					//data value
					uint32 type;
					std::string nativeTypeTemp;
					ExprPushValue(member.GetToken(), type, nativeTypeTemp);
					typeStack.push_back(type);
					nativeTypeStack.push_back(nativeTypeTemp);
				}
				else if (IsUnaryOperation(member.GetToken().GetType()))
				{
					//make sure that the number of operands is correct
					if (typeStack.empty())
						throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));

					//unary minus
					if (member.GetToken().GetType() == TOKEN_UNARY_MINUS)
					{
						uint32 operandType = typeStack.back();
						if (operandType == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_NEGF));
						}
						else if (operandType == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_NEGI));
						}
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
					}
					// not
					else if (member.GetToken().GetType() == TOKEN_NOT)
					{
						uint32 operandType = typeStack.back();
						if (operandType == dsr::VMDATATYPE_BOOL)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_NOT));
						}
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
					}
					else
					{
						throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
					}
				}
				else if (IsBinaryOperation(member.GetToken().GetType()))
				{
					//make sure that the number of operands is correct
					if (typeStack.size() < 2)
						throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));

					//get operand types
					const uint32 op2Type = typeStack.back();
					typeStack.pop_back();
					nativeTypeStack.pop_back();
					const uint32 op1Type = typeStack.back();
					typeStack.pop_back();
					nativeTypeStack.pop_back();

					//ops
					if (member.GetToken().GetType() == TOKEN_DIVIDE)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_DIVII));
							typeStack.push_back(dsr::VMDATATYPE_INT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_DIVFF));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_DIVIF));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_DIVFI));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
					}
					else if (member.GetToken().GetType() == TOKEN_MULTIPLY)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_MULII));
							typeStack.push_back(dsr::VMDATATYPE_INT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_MULFF));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_MULIF));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_MULFI));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
					}
					else if (member.GetToken().GetType() == TOKEN_MINUS)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_SUBII));
							typeStack.push_back(dsr::VMDATATYPE_INT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_SUBFF));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_SUBIF));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_SUBFI));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
					}
					else if (member.GetToken().GetType() == TOKEN_PLUS)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_ADDII));
							typeStack.push_back(dsr::VMDATATYPE_INT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_ADDFF));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_ADDIF));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_ADDFI));
							typeStack.push_back(dsr::VMDATATYPE_FLOAT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
					}
					else if (member.GetToken().GetType() == TOKEN_MODULO)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
						{
							m_curCode.push_back(BuildCode(dsr::VMI_MOD));
							typeStack.push_back(dsr::VMDATATYPE_INT);
							nativeTypeStack.push_back("");
							DecStackSize();
						}
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
					}
					else if (member.GetToken().GetType() == TOKEN_EQUALS)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_EQII));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_EQFF));
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_EQIF));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_EQFI));
						else if (op1Type == dsr::VMDATATYPE_BOOL && op2Type == dsr::VMDATATYPE_BOOL)
							m_curCode.push_back(BuildCode(dsr::VMI_EQBB));
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
						typeStack.push_back(dsr::VMDATATYPE_BOOL);
						nativeTypeStack.push_back("");
						DecStackSize();
					}
					else if (member.GetToken().GetType() == TOKEN_NOT_EQUALS)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_EQII));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_EQFF));
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_EQIF));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_EQFI));
						else if (op1Type == dsr::VMDATATYPE_BOOL && op2Type == dsr::VMDATATYPE_BOOL)
							m_curCode.push_back(BuildCode(dsr::VMI_EQBB));
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
						m_curCode.push_back(BuildCode(dsr::VMI_NOT));
						typeStack.push_back(dsr::VMDATATYPE_BOOL);
						nativeTypeStack.push_back("");
						DecStackSize();
					}
					else if (member.GetToken().GetType() == TOKEN_LTEQ)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_LTEQII));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_LTEQFF));
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_LTEQIF));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_LTEQFI));
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
						typeStack.push_back(dsr::VMDATATYPE_BOOL);
						nativeTypeStack.push_back("");
						DecStackSize();
					}
					else if (member.GetToken().GetType() == TOKEN_LT)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_LTII));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_LTFF));
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_LTIF));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_LTFI));
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
						typeStack.push_back(dsr::VMDATATYPE_BOOL);
						nativeTypeStack.push_back("");
						DecStackSize();
					}
					else if (member.GetToken().GetType() == TOKEN_GTEQ)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_GTEQII));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_GTEQFF));
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_GTEQIF));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_GTEQFI));
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
						typeStack.push_back(dsr::VMDATATYPE_BOOL);
						nativeTypeStack.push_back("");
						DecStackSize();
					}
					else if (member.GetToken().GetType() == TOKEN_GT)
					{
						if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_GTII));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_GTFF));
						else if (op1Type == dsr::VMDATATYPE_INT && op2Type == dsr::VMDATATYPE_FLOAT)
							m_curCode.push_back(BuildCode(dsr::VMI_GTIF));
						else if (op1Type == dsr::VMDATATYPE_FLOAT && op2Type == dsr::VMDATATYPE_INT)
							m_curCode.push_back(BuildCode(dsr::VMI_GTFI));
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
						typeStack.push_back(dsr::VMDATATYPE_BOOL);
						nativeTypeStack.push_back("");
						DecStackSize();
					}
					else if (member.GetToken().GetType() == TOKEN_AND)
					{
						if (op1Type == dsr::VMDATATYPE_BOOL && op2Type == dsr::VMDATATYPE_BOOL)
							m_curCode.push_back(BuildCode(dsr::VMI_AND));
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
						typeStack.push_back(dsr::VMDATATYPE_BOOL);
						nativeTypeStack.push_back("");
						DecStackSize();
					}
					else if (member.GetToken().GetType() == TOKEN_OR)
					{
						if (op1Type == dsr::VMDATATYPE_BOOL && op2Type == dsr::VMDATATYPE_BOOL)
							m_curCode.push_back(BuildCode(dsr::VMI_OR));
						else
						{
							throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
						}
						typeStack.push_back(dsr::VMDATATYPE_BOOL);
						nativeTypeStack.push_back("");
						DecStackSize();
					}
					else
					{
						throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
					}
				}
				else
				{
					throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.", m_pDeclaration->GetName(), expr.GetLine()));
				}
			}
		}

		assert(typeStack.size() == nativeTypeStack.size());
		if (typeStack.size() != 1)
		{
			throw CompilerException(FORMAT("Invalid expression.  Class %s, line %u.",
				m_pDeclaration->GetName(), expr.GetLine()));
		}

		retType = typeStack.back();
		nativeRetType = nativeTypeStack.back();
	}

	bool Compiler::IsA(const char* derived, const char* base) const
	{
		if (strlen(derived) == 0 || strlen(base) == 0)
			return false;

		const char* der = derived;
		while (strlen(der) != 0)
		{
			if (strcmp(der, base) == 0)
				return true;

			const ScriptClassDeclaration* pDer = GetScriptClassDeclarationPtr(derived);
			der = pDer->GetSuperClassName();
		}

		return false;
	}

	ScriptSourceCPtr Compiler::LoadScriptSource(const char* scriptName)
	{
		if (GetNumPaths() == 0)
		{
			assert(false);
			throw CompilerException(FORMAT("Must have at least one script path defined."));
		}

		std::string scriptFileName = GenerateScriptFileName(scriptName);

		std::string data;
		for (uint32 i=0; i<GetNumPaths() && data.empty(); ++i)
		{
			const char* path = GetPath(i);
			File::StringVector foundFiles;
			FindFiles(path, scriptFileName.c_str(), foundFiles);
			if (foundFiles.size() == 1)
			{
				LoadScriptFile(foundFiles[0].c_str(), data);
			}
			else if (foundFiles.size() > 1)
			{
				assert(false);
				throw CompilerException(FORMAT("Can't use wildcards in Compiler::BuildClass()."));
			}
		}

		if (data.empty())
		{
			assert(false);
			throw CompilerException(FORMAT("Could not find the file %s.", scriptFileName.c_str()));
		}

		Parser parser;
		ScriptSourceCPtr scriptSource = parser.ParseScript(data.c_str());
		if (strcmp(scriptSource->GetName(), scriptName) != 0)
		{
			throw CompilerException(
				FORMAT("Trying to load script %s, but file contains script %s.",
				scriptName, scriptSource->GetName()));
		}

		return scriptSource;
	}

	void Compiler::LoadAllScriptSources(const char* name)
	{
		m_curCompilePass = COMPILEPASS_LOADSCRIPTSOURCES;

		for (ScriptSourceCPtrList::const_iterator itt = m_sources.begin();
				itt != m_sources.end();
				++itt)
		{
			if (strcmp((*itt)->GetName(), name) == 0)
				return;
		}

		ScriptSourceCPtr scriptSource = LoadScriptSource(name);
		m_sources.push_back(scriptSource);

		for (uint32 i=0; i<scriptSource->GetNumImportClasses(); ++i)
		{
			LoadAllScriptSources(scriptSource->GetImportClass(i));
		}
	}

	void Compiler::BuildScriptClassDeclarationsPass1()
	{
		m_curCompilePass = COMPILEPASS_BUILDDECLARATIONS1;

		for (ScriptSourceCPtrList::const_iterator itt = m_sources.begin();
				itt != m_sources.end(); ++itt)
		{
			const ScriptSourceCPtr source = *itt;
			ScriptClassDeclarationCPtr decl = BuildScriptClassDeclarationPass1(*source);
			m_declarationList.push_back(decl);
		}
	}

	void Compiler::BuildScriptClassDeclarationsPass2()
	{
		m_curCompilePass = COMPILEPASS_BUILDDECLARATIONS2;

		for (ScriptClassDeclarationCPtrList::iterator it = m_declarationList.begin();
				it != m_declarationList.end(); ++it)
		{
			ScriptClassDeclaration* pDecl = *it;
			BuildScriptClassDeclarationPass2(pDecl);
		}
	}

	void Compiler::CheckDataDeclarations(const ScriptClassDeclaration* pDecl)
	{
		for (uint32 i=0; i<pDecl->GetNumData(); ++i)
		{
			//check for duplicates
			const DataDeclaration* pData = pDecl->GetDataDeclarationPtr(i);
			for (uint32 j=i+1; j<pDecl->GetNumData(); ++j)
			{
				const DataDeclaration* pData2 = pDecl->GetDataDeclarationPtr(j);
				if (strcmp(pData->GetName(), pData2->GetName()) == 0)
				{
					throw CompilerException(FORMAT("Data member \"%s\" is a duplicate.  Class %s, line %u.",
						pData->GetName(), pDecl->GetName(), pData->GetLine()));
				}
			}

			//check types
			if (pData->GetType() == dsr::VMDATATYPE_NATIVE)
			{
				//make sure that the script class is loaded
				GetScriptClassDeclarationPtr(pData->GetNativeType());
			}
			else if (pData->GetType() == dsr::VMDATATYPE_VOID || pData->GetType() == dsr::VMDATATYPE_MAX)
				throw CompilerException(FORMAT("Internal compiler error. File %s, line %u.", __FILE__, __LINE__));
		}
	}

	void Compiler::CheckFunctionDeclarations(ScriptClassDeclaration* pDecl)
	{
		std::list<uint32> idxToRemove;
		for (uint32 i=0; i<pDecl->GetNumNonSuperFunctions(); ++i)
		{
			const FunctionDeclaration* pC1 = pDecl->GetNonSuperFunctionDeclarationPtr(i);

			//check for duplicate functions (internal)
			for (uint32 j=i+1; j<pDecl->GetNumNonSuperFunctions(); ++j)
			{
				const FunctionDeclaration* pC2 = pDecl->GetNonSuperFunctionDeclarationPtr(j);
				if (strcmp(pC1->GetName(), pC2->GetName()) == 0)
				{
					throw CompilerException(FORMAT("Function member \"%s\" is a duplicate.  Class %s, line %u.",
						pC1->GetName(), pDecl->GetName(), pC2->GetLine()));
				}
			}

			//remove overriden function declarations & make sure params match
			for (uint32 j=0; j<pDecl->GetNumFunctions() - pDecl->GetNumNonSuperFunctions(); ++j)
			{
				const FunctionDeclaration* pC2 = pDecl->GetFunctionDeclarationPtr(j);
				if (strcmp(pC1->GetName(), pC2->GetName()) == 0)
				{
					//check ret type
					if (pC1->GetReturnType() != pC2->GetReturnType())
					{
						throw CompilerException(FORMAT("Return type of overriding function \"%s\" does not match.  Class %s, line %u.",
							pC1->GetName(), pDecl->GetName(), pC1->GetLine()));
					}
					if (pC1->GetReturnType() == dsr::VMDATATYPE_NATIVE
						&& strcmp(pC1->GetNativeReturnType(), pC2->GetNativeReturnType()) != 0)
					{
						throw CompilerException(FORMAT("Return type of overriding function \"%s\" does not match.  Class %s, line %u.",
							pC1->GetName(), pDecl->GetName(), pC1->GetLine()));
					}

					//check num params
					if (pC1->GetNumParameters() != pC2->GetNumParameters())
					{
						throw CompilerException(FORMAT("Number of parameters of the overriding function \"%s\" does not match.  Class %s, line %u.",
							pC1->GetName(), pDecl->GetName(), pC1->GetLine()));
					}

					//check param types
					for (uint32 k=0; k<pC1->GetNumParameters(); ++k)
					{
						const DataDeclaration* pParam1 = pC1->GetParameterDataDeclarationPtr(k);
						const DataDeclaration* pParam2 = pC2->GetParameterDataDeclarationPtr(k);
						if (pParam1->GetType() != pParam2->GetType())
						{
							throw CompilerException(FORMAT("Parameter type \"%s\"of overriding function \"%s\" does not match.  Class %s, line %u.",
								pParam1->GetName(), pC1->GetName(), pDecl->GetName(), pC1->GetLine()));
						}
						if (pParam1->GetType() == dsr::VMDATATYPE_NATIVE
							&& strcmp(pParam1->GetNativeType(), pParam2->GetNativeType()) != 0)
						{
							throw CompilerException(FORMAT("Parameter type \"%s\"of overriding function \"%s\" does not match.  Class %s, line %u.",
								pParam1->GetName(), pC1->GetName(), pDecl->GetName(), pC1->GetLine()));
						}
					}

					idxToRemove.push_back(i);
				}
			}
		}

		//remove overriden functions
		while (!idxToRemove.empty())
		{
			uint32 idx = idxToRemove.back();
			idxToRemove.pop_back();
			pDecl->RemoveNonSuperFunctionDeclaration(idx);
		}

		//check data types
		for (uint32 i=0; i<pDecl->GetNumFunctions(); ++i)
		{
			const FunctionDeclaration* pC1 = pDecl->GetFunctionDeclarationPtr(i);

			//check return type
			if (pC1->GetReturnType() == dsr::VMDATATYPE_NATIVE)
				GetScriptClassDeclarationPtr(pC1->GetNativeReturnType());

			//check param data types
			for (uint32 j=0; j<pC1->GetNumParameters(); ++j)
			{
				const DataDeclaration* pP1 = pC1->GetParameterDataDeclarationPtr(j);
				if (pP1->GetType() == dsr::VMDATATYPE_NATIVE)
				{
					//make sure that the script class is loaded
					GetScriptClassDeclarationPtr(pP1->GetNativeType());
				}
			}
		}
	}

	void Compiler::CheckConstructorDeclarations(ScriptClassDeclaration* pDecl)
	{
		for (uint32 i=0; i<pDecl->GetNumConstructors(); ++i)
		{
			const FunctionDeclaration* pC1 = pDecl->GetConstructorFunctionDeclarationPtr(i);

			//check param data types
			for (uint32 j=0; j<pC1->GetNumParameters(); ++j)
			{
				const DataDeclaration* pP1 = pC1->GetParameterDataDeclarationPtr(j);
				if (pP1->GetType() == dsr::VMDATATYPE_NATIVE)
				{
					//make sure that the script class is loaded
					GetScriptClassDeclarationPtr(pP1->GetNativeType());
				}
			}

			//check for duplicate or ambiguous parameters
			for (uint32 j=i+1; j<pDecl->GetNumConstructors(); ++j)
			{
				const FunctionDeclaration* pC2 = pDecl->GetConstructorFunctionDeclarationPtr(j);
				if (pC1->GetNumParameters() == pC2->GetNumParameters())
				{
					bool same = true;
					for (uint32 k=0; k<pC1->GetNumParameters(); ++k)
					{
						const DataDeclaration* pP1 = pC1->GetParameterDataDeclarationPtr(k);
						const DataDeclaration* pP2 = pC2->GetParameterDataDeclarationPtr(k);
						if (pP1->GetType() != pP2->GetType())
						{
							same = false;
						}
						else if (pP1->GetType() == dsr::VMDATATYPE_NATIVE)
						{
							const ScriptClassDeclaration* pNT1 = GetScriptClassDeclarationPtr(pP1->GetNativeType());
							const ScriptClassDeclaration* pNT2 = GetScriptClassDeclarationPtr(pP2->GetNativeType());
							if (pNT1 != pNT2 && !IsA(pNT1->GetName(), pNT2->GetName()) && !IsA(pNT2->GetName(), pNT1->GetName()))
								same = false;
						}
					}

					if (same)
					{
						throw CompilerException(FORMAT("Class has a duplicate or ambiguous constructor.  Class %s, line %u.", pDecl->GetName(), pC2->GetLine()));
					}
				}
			}
		}
	}

	void Compiler::BuildScriptClassDeclarationPass2(ScriptClassDeclaration* pDecl)
	{
		CheckDataDeclarations(pDecl);
		CheckConstructorDeclarations(pDecl);
		CheckFunctionDeclarations(pDecl);
	}

	ScriptClassCPtr Compiler::BuildScriptClassImplementation(const char* scriptName)
	{
		//set compile context
		m_curCompilePass = COMPILEPASS_BUILDFUNCTIONIMPLEMENTATIONS;
		m_pDeclaration = GetScriptClassDeclarationPtr(scriptName);
		const ScriptSource* pScriptSource = *(std::find_if(m_sources.begin(), m_sources.end(), FindByName(scriptName)));
		assert(m_pDeclaration);
		assert(pScriptSource);

		ScriptClassCPtr res = new ScriptClass();
		res->SetScriptDeclaration(m_pDeclaration);

		//build functions
		for (uint32 i=0; i<pScriptSource->GetNumFunctions(); ++i)
		{
			const FunctionSrc* pFuncSrc = pScriptSource->GetFunctionSrcPtr(i);
			m_curFuncIdx = m_pDeclaration->GetFunctionIndex(pFuncSrc->GetName());
			assert(m_curFuncIdx != -1);
			FunctionImplementationCPtr pFuncImpl = BuildFunctionImplementation(*pFuncSrc);
			res->AddFunctionImplementation(pFuncImpl);
		}
		m_curFuncIdx = -1;

		//build constructors
		for (uint32 i=0; i<pScriptSource->GetNumConstructors(); ++i)
		{
			const FunctionSrc* pFuncSrc = pScriptSource->GetConstructorFunctionSrcPtr(i);
			m_curCtorIdx = i;
			FunctionImplementationCPtr pFuncImpl = BuildFunctionImplementation(*pFuncSrc);
			res->AddConstructorImplementation(pFuncImpl);
		}
		m_curCtorIdx = -1;

		return res;
	}

	ScriptClassCPtr Compiler::BuildScriptClass(const char* scriptName)
	{
		try
		{
			Clear();
			LoadAllScriptSources(scriptName);
			BuildScriptClassDeclarationsPass1();
			BuildScriptClassDeclarationsPass2();
			return BuildScriptClassImplementation(scriptName);
		}
		catch (const CompilerException& e)
		{
			if (strlen(e.GetError()) > 0)
				m_error = e.GetError();

			return 0;
		}
	}

	void Compiler::GetVarInfo(const char* vname, VarInfo& varInfo)
	{
		//look for data in the parameter list
		const FunctionDeclaration* pFuncDecl = GetFunctionDeclarationPtr(m_pDeclaration, m_curFuncIdx, m_curCtorIdx);
		for (uint32 i=0; i<pFuncDecl->GetNumParameters(); ++i)
		{
			const DataDeclaration* pParam = pFuncDecl->GetParameterDataDeclarationPtr(i);
			if (strcmp(pParam->GetName(), vname) == 0)
			{
				//found
				varInfo.Set(VarInfo::DATALOC_PARAMETER, i, pParam->GetType(), pParam->GetNativeType());
				return;
			}
		}

		//look for the variable in local data
		assert(m_pCurFuncImpl);
		for (uint32 i=0; i<m_pCurFuncImpl->GetNumLocals(); ++i)
		{
			const DataDeclaration* pLocal = m_pCurFuncImpl->GetLocalDataDeclarationPtr(i);
			if (strcmp(pLocal->GetName(), vname) == 0)
			{
				//found
				varInfo.Set(VarInfo::DATALOC_LOCAL, i, pLocal->GetType(), pLocal->GetNativeType());
				return;
			}
		}

		//look for the variable in the script data
		for (uint32 i=0; i<m_pDeclaration->GetNumData(); ++i)
		{
			const DataDeclaration* pData = m_pDeclaration->GetDataDeclarationPtr(i);
			if (strcmp(pData->GetName(), vname) == 0)
			{
				//found
				varInfo.Set(VarInfo::DATALOC_SCRIPT, i, pData->GetType(), pData->GetNativeType());
				return;
			}
		}

		throw CompilerException(FORMAT("Could not find variable %s.  Class %s.",
			vname, m_pDeclaration->GetName()));
	}

	const ScriptClassDeclaration* Compiler::GetScriptClassDeclarationPtr(const char* className) const
	{
		assert(m_curCompilePass == COMPILEPASS_BUILDFUNCTIONIMPLEMENTATIONS
			|| m_curCompilePass == COMPILEPASS_BUILDDECLARATIONS2);

		for (ScriptClassDeclarationCPtrList::const_iterator it = m_declarationList.begin();
				it != m_declarationList.end(); ++it)
		{
			const ScriptClassDeclaration* pDecl = *it;
			if (strcmp(className, pDecl->GetName()) == 0)
				return pDecl;
		}

		assert(false);
		throw CompilerException(FORMAT("Could not find class %s.", className));
		return 0;
	}

	DataDeclarationCPtr Compiler::BuildDataDeclaration(const FunctionParameterSrc& source) const
	{
		DataDeclarationCPtr dataDecl = new DataDeclaration();
		dataDecl->SetLine(source.GetLine());
		dataDecl->SetName(source.GetName());
		dataDecl->SetType(GetDataType(source.GetType()));
		if (dataDecl->GetType() == dsr::VMDATATYPE_NATIVE)
			dataDecl->SetNativeType(source.GetType());
		else
			dataDecl->SetNativeType("");
		return dataDecl;
	}

	DataDeclarationCPtr Compiler::BuildDataDeclaration(const DataSrc& source) const
	{
		DataDeclarationCPtr dataDecl = new DataDeclaration();
		dataDecl->SetLine(source.GetLine());
		dataDecl->SetName(source.GetName());
		dataDecl->SetType(GetDataType(source.GetType()));
		if (dataDecl->GetType() == dsr::VMDATATYPE_NATIVE)
			dataDecl->SetNativeType(source.GetType());
		else
			dataDecl->SetNativeType("");
		return dataDecl;
	}

	FunctionDeclarationCPtr Compiler::BuildFunctionDeclaration(const FunctionSrc& source) const
	{
		FunctionDeclarationCPtr funcDecl = new FunctionDeclaration();
		funcDecl->SetLine(source.GetLine());
		funcDecl->SetName(source.GetName());
		funcDecl->SetReturnType(GetDataType(source.GetReturnType()));
		if (funcDecl->GetReturnType() == dsr::VMDATATYPE_NATIVE)
			funcDecl->SetReturnNativeType(source.GetReturnType());
		else
			funcDecl->SetReturnNativeType("");

		for (uint32 i=0; i<source.GetNumParameters(); ++i)
			funcDecl->AddParameter(BuildDataDeclaration(*(source.GetFunctionParameterSrcPtr(i))));

		return funcDecl;
	}

	FunctionImplementationCPtr Compiler::BuildFunctionImplementation(const FunctionSrc& source)
	{
		const FunctionDeclaration* pFuncDecl = GetFunctionDeclarationPtr(m_pDeclaration, m_curFuncIdx, m_curCtorIdx);
		FunctionImplementationCPtr funcImpl = new FunctionImplementation();
		m_pCurFuncImpl = funcImpl;

		//get implementation
		if (source.IsNative())
		{
			//native function implementation
			ClearCurCode();
		}
		else
		{
			//compile local data
			for (uint32 i=0; i<source.GetNumLocals(); ++i)
			{
				const DataSrc* pDataSrc = source.GetLocalDataSrcPtr(i);
				DataDeclarationCPtr dataDecl = BuildDataDeclaration(*pDataSrc);
				funcImpl->AddLocalDataDeclaration(dataDecl);
			}

			//non-native function implementation
			ClearCurCode();

			//call base constructor
			if (source.IsConstructor() && strlen(m_pDeclaration->GetSuperClassName()) > 0)
			{
				uint32 rt;
				std::string nrt;
				const FunctionCallSrc* pBaseCtor = source.GetBaseConstructor();
				assert(pBaseCtor);
				VisitFunctionCall(*pBaseCtor, rt, nrt, "");
				assert(m_curStackSize == 0);
			}

			//initialise data members
			for (uint32 i=0; i<source.GetNumLocals(); ++i)
			{
				const DataSrc* pDataSrc = source.GetLocalDataSrcPtr(i);
				const DataDeclaration* pDataDecl = funcImpl->GetLocalDataDeclarationPtr(i);
				assert(pDataDecl);
				const ExpressionSrc* pExpr = pDataSrc->GetExpressionSrcPtr();
				if (pExpr)
				{
					VarInfo varInfo;
					varInfo.Set(Compiler::VarInfo::DATALOC_SCRIPT, i, pDataDecl->GetType(), pDataDecl->GetNativeType());
					VisitExpressionAndCheckRetTypes(*pExpr, pDataDecl->GetType(), pDataDecl->GetNativeType());
					AssignToVariable(varInfo);

					assert(m_curStackSize == 0);
				}
			}

			//visit function body
			if (source.GetStatementSrcPtr())
				source.GetStatementSrcPtr()->Visit(*this);

			//create fake return value for void functions
			if (pFuncDecl->GetReturnType() == dsr::VMDATATYPE_VOID && (m_curCode.empty() || ExtractVMInstruction(m_curCode.back()) != dsr::VMI_RET))
			{
				m_curCode.push_back(BuildCode(dsr::VMI_PUSHB));
				IncStackSize();
				m_curCode.push_back(BuildCode(dsr::VMI_RET));
				DecStackSize();
			}

			//check for return
			if (m_curCode.empty() || ExtractVMInstruction(m_curCode.back()) != dsr::VMI_RET)
				throw CompilerException(FORMAT("Function \"%s\" must return a value.  Class %s, line %u.", source.GetName(), m_pDeclaration->GetName(), source.GetLine()));
		}

		//get bytecode
		funcImpl->SetVMCodeBlock(m_curCode);
		assert(m_curStackSize == 0);
		funcImpl->SetMaxStackSize(m_maxStackSize);

		m_pCurFuncImpl = 0;
		return funcImpl;
	}
}