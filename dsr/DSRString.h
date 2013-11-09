// (c) 2004 DodoSoft Inc.
#if !defined(DSR_STRING_H_)
#define DSR_STRING_H_

#include <string.h>
#include "DSRPlatform.h"
#include "DSRClassUtils.h"
#include "DSRMemory.h"

namespace dsr
{
	class String
	{
		DSR_NOCOPY(String)
	public:
		DSR_NEWDELETE(String)

		String()
		: m_str(0)
		{
		}

		explicit String(const char* str)
		{
			const size_t len = strlen(str);
			m_str = (char*) Memory::Alloc(len+1);
			strcpy(m_str, str);
		}

		~String()
		{
			Memory::Free(m_str);
		}

		const char* c_str() const
		{
			return m_str;
		}

	private:
		char* m_str;
	};
}

#endif