// (c) 2004 DodoSoft Inc.
#if !defined(DSR_PLATFORM_H_)
#define DSR_PLATFORM_H_

#include <cassert>

#define DSR_TARGET_GC 1
#define DSR_TARGET_PS2 2
#define DSR_TARGET_XBOX 3
#define DSR_TARGET_PC 4

#define DSR_TARGET DSR_TARGET_PC

#if DSR_TARGET == DSR_TARGET_GC
	#define DSR_INLINE inline
	#define DSR_ASSERT(val) assert(val)
#elif DSR_TARGET == DSR_TARGET_PS2
	#define DSR_INLINE static inline
	#define DSR_ASSERT(val) assert(val)
#elif DSR_TARGET == DSR_TARGET_XBOX
	#define DSR_INLINE __inline
	#define DSR_ASSERT(val) assert(val)
	//make standard for
	#undef for
	#define for if (0){}else for
#elif DSR_TARGET == DSR_TARGET_PC
	#define DSR_INLINE __inline
	#define DSR_ASSERT(val) assert(val)
	//make standard for
	#undef for
	#define for	if (0){}else for
#endif

#endif