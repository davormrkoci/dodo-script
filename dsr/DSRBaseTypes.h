
#if !defined(DSR_BASETYPES_H_)
#define DSR_BASETYPES_H_

#include "DSRPlatform.h"

namespace dsr
{
#if DSR_TARGET == DSR_TARGET_GC
	typedef unsigned long long	uint64;
	typedef unsigned long		uint32;
	typedef unsigned short		uint16;
	typedef unsigned char		uint8;
	typedef unsigned char		ubyte;
	typedef signed long long	int64;
	typedef signed long			int32;
	typedef signed short		int16;
	typedef signed char			int8;
	typedef float				float32;
	#define DSR_INLINE inline
#elif DSR_TARGET == DSR_TARGET_PS2
	typedef unsigned long		uint64;
	typedef unsigned int		uint32;
	typedef unsigned short		uint16;
	typedef unsigned char		uint8;
	typedef unsigned char		ubyte;
	typedef signed long			int64;
	typedef signed int			int32;
	typedef signed short		int16;
	typedef signed char			int8;
	typedef float				float32;
	#define DSR_INLINE static inline
#elif DSR_TARGET == DSR_TARGET_XBOX
	typedef unsigned long		uint64;
	typedef unsigned int		uint32;
	typedef unsigned short		uint16;
	typedef unsigned char		uint8;
	typedef unsigned char		ubyte;
	typedef signed long			int64;
	typedef signed int			int32;
	typedef signed short		int16;
	typedef signed char			int8;
	typedef float				float32;
	#define DSR_INLINE __inline
#elif DSR_TARGET == DSR_TARGET_PC
	typedef unsigned __int64	uint64;
	typedef unsigned int		uint32;
	typedef unsigned short		uint16;
	typedef unsigned char		uint8;
	typedef unsigned char		ubyte;
	typedef signed __int64		int64;
	typedef signed int			int32;
	typedef signed short		int16;
	typedef signed char			int8;
	typedef float				float32;
	#define DSR_INLINE __inline
#endif

	//endian conversion
	#define DSR_SWAPENDIAN_16(i) ((((uint16)(i)) & 0xff00) >> 8 | (((uint16)(i)) & 0x00ff) << 8)
	#define DSR_SWAPENDIAN_32(i) ((((uint32)(i)) & 0xff000000) >> 24 | (((uint32)(i)) & 0x00ff0000) >> 8 | (((uint32)(i)) & 0x0000ff00) << 8 | (((uint32)(i)) & 0x000000ff) << 24)

	DSR_INLINE void DSR_SwapEndianInplace_16(void* p16)
	{
		*((uint16*)p16) = (uint16) DSR_SWAPENDIAN_16(*((uint16*)p16));
	}

	DSR_INLINE void DSR_SwapEndianInplace_32(void* p32)
	{
		*((uint32*)p32) = (uint32) DSR_SWAPENDIAN_32(*((uint32*)p32));
	}
}

#endif
