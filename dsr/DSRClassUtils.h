// (c) 2004 DodoSoft Inc.
#if !defined(DSR_CLASSUTILS_H_)
#define DSR_CLASSUTILS_H_

#include "DSRPlatform.h"

//macro for stubbing out copy constructor and assignment operator
#define DSR_NOCOPY(className)					\
	private:									\
		className(const className&);			\
		className& operator=(const className&);	\

#endif