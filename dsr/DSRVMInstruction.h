// (c) 2004 DodoSoft Inc.
#if !defined(DSR_VMINSTRUCTION_H_)
#define DSR_VMINSTRUCTION_H_

#include "DSRPlatform.h"
#include "DSRBaseTypes.h"
#include "DSRArray.h"

namespace dsr
{
	enum
	{
		VMI_INVALID = 0,			//should never occur at runtime
		VMI_NOP,					//do nothing, used by debbuger
		VMI_CALLF_SELF_G,			// <Y> call function Y in script self
		VMI_CALLF_SUPER_G,			// <Y> call function Y in super script
		VMI_CALLF_PUSHED_G,			// <Y> call global function Y in script that is on top of the stack
		VMI_CALLC_PUSHED_G,			// <Y> call constructor Y in script that is on top of the stack
		VMI_CALLC_SELF_SUPER,		// <Y> call constructor in super script
		VMI_RET,					//return from a function or a sequence
		VMI_JMP,					//00xxxxxx absolute jump to address x
		VMI_JZ,						//00xxxxxx if pop stack is 0, absolute jump to address x
		VMI_STORESF,				//00xxxxxx pop float from stack + move popped value to script data[x]
		VMI_STORESI,				//00xxxxxx pop integer from stack + move popped value to script data[x]
		VMI_STORESB,				//00xxxxxx pop bool from stack + move popped value to script data[x]
		VMI_STORESN,				//00xxxxxx pop native type from stack + move popped value to script data[x]
		VMI_STORELF,				//00xxxxxx pop float from stack + move popped value to local data[x]
		VMI_STORELI,				//00xxxxxx pop integer from stack + move popped value to local data[x]
		VMI_STORELB,				//00xxxxxx pop bool from stack + move popped value to local data[x]
		VMI_STORELN,				//00xxxxxx pop native type from stack + move popped value to local data[x]
		VMI_STOREPF,				//00xxxxxx pop float from stack + move popped value to parameter data[x]
		VMI_STOREPI,				//00xxxxxx pop integer from stack + move popped value to parameter data[x]
		VMI_STOREPB,				//00xxxxxx pop bool from stack + move popped value to parameter data[x]
		VMI_STOREPN,				//00xxxxxx pop native type from stack + move popped value to parameter data[x]
		VMI_FETCHSF,				//00xxxxxx push onto stack float value of script data[x]
		VMI_FETCHSI,				//00xxxxxx push onto stack int value of script data[x]
		VMI_FETCHSB,				//00xxxxxx push onto stack bool value of script data[x]
		VMI_FETCHSN,				//00xxxxxx push onto stack native value of script data[x]
		VMI_FETCHLF,				//00xxxxxx push onto stack float value of local data[x]
		VMI_FETCHLI,				//00xxxxxx push onto stack int value of local data[x]
		VMI_FETCHLB,				//00xxxxxx push onto stack bool value of local data[x]
		VMI_FETCHLN,				//00xxxxxx push onto stack native value of local data[x]
		VMI_FETCHPF,				//00xxxxxx push onto stack float value of parameter data[x]
		VMI_FETCHPI,				//00xxxxxx push onto stack int value of parameter data[x]
		VMI_FETCHPB,				//00xxxxxx push onto stack bool value of parameter data[x]
		VMI_FETCHPN,				//00xxxxxx push onto stack native value of parameter data[x]
		VMI_PUSHF,					// <X> push float value x onto the stack
		VMI_PUSHI,					// <X> push large int value x onto the stack
		VMI_PUSHB,					//00xxxxxx push bool value x onto the stack
		VMI_POP,					//pop the top of the stack
		VMI_NEGF,					//negate the float on top of the stack
		VMI_NEGI,					//negate the integer on top of the stack
		VMI_NOT,					//not the top of the stack
		VMI_DIVII,					//divide I,I
		VMI_DIVFF,					//divide F,F
		VMI_DIVFI,					//divide F,I
		VMI_DIVIF,					//divide I,F
		VMI_MULII,					//multiply I,I
		VMI_MULFF,					//multiply F,F
		VMI_MULFI,					//multiply F,I
		VMI_MULIF,					//multiply I,F
		VMI_SUBII,					//subtract I,I
		VMI_SUBFF,					//subtract F,F
		VMI_SUBFI,					//subtract F,I
		VMI_SUBIF,					//subtract I,F
		VMI_ADDII,					//add I,I
		VMI_ADDFF,					//add F,F
		VMI_ADDFI,					//add F,I
		VMI_ADDIF,					//add I,F
		VMI_MOD,					//mod I,I
		VMI_EQII,					//I == I
		VMI_EQFF,					//F == F
		VMI_EQFI,					//F == I
		VMI_EQIF,					//I == F
		VMI_EQBB,					//B == B
		VMI_LTEQII,					//I <= I
		VMI_LTEQFF,					//F <= F
		VMI_LTEQFI,					//F <= I
		VMI_LTEQIF,					//I <= F
		VMI_LTII,					//I < I
		VMI_LTFF,					//F < F
		VMI_LTFI,					//F < I
		VMI_LTIF,					//I < F
		VMI_GTEQII,					//I >= I
		VMI_GTEQFF,					//F >= F
		VMI_GTEQFI,					//F >= I
		VMI_GTEQIF,					//I >= F
		VMI_GTII,					//I > I
		VMI_GTFF,					//F > F
		VMI_GTFI,					//F > I
		VMI_GTIF,					//I > F
		VMI_AND,					//B && B
		VMI_OR,						//B || B
		VMI_NEW						//00xxxxxx create instance of of type newstring[x]
	};
	
	typedef uint8 VMInstruction;
	typedef uint32 VMBytecode;
	typedef Array<VMBytecode> VMCodeBlock;
}

#endif