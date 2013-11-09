// (c) 2004 DodoSoft Inc.
#if !defined(DSC_CLASSUTILS_H_)
#define DSC_CLASSUTILS_H_

//macro for stubbing out copy constructor and assignment operator
#define DSC_NOCOPY(className)					\
	private:									\
		className(const className&);			\
		className& operator=(const className&);	\

#endif