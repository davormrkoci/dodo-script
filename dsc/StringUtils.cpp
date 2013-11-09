#include "StringUtils.h"
#include "BaseTypes.h"

namespace dsc
{
	//--------------------------------------------------------------------
	const std::string ToString(double d, int nDigits)
	{
		char buf[256];
		return std::string(_gcvt(d, nDigits, buf));
	}
	
	//--------------------------------------------------------------------
	const std::string ToString(float f, int nDigits)
	{
		char buf[256];
		return std::string(_gcvt(f, nDigits, buf));
	}
	
	//--------------------------------------------------------------------
	const std::string ToString(long l, int radix)
	{
		char buf[256];
		return std::string(_ltoa(l, buf, radix));
	}
	
	//--------------------------------------------------------------------
	const std::string ToString(unsigned long ul, int radix)
	{
		char buf[256];
		return std::string(_ultoa(ul, buf, radix));
	}
	
	//--------------------------------------------------------------------
	const std::string ToString(int i, int radix)
	{
		char buf[256];
		return std::string(_itoa(i, buf, radix));
	}
	
	//--------------------------------------------------------------------
	const std::string ToString(unsigned int ui, int radix)
	{
		char buf[256];
		return std::string(_ultoa(ui, buf, radix));
	}
	
	//--------------------------------------------------------------------
	const std::string ToString(short s, int radix)
	{
		char buf[256];
		return std::string(_itoa(s, buf, radix));
	}
	
	//--------------------------------------------------------------------
	const std::string ToString(unsigned short us, int radix)
	{
		char buf[256];
		return std::string(_itoa(us, buf, radix));
	}

	//--------------------------------------------------------------------
	const std::string ToString(bool b)
	{
		if (b)
			return "true";
		else
			return "false";
	}

	//--------------------------------------------------------------------
	void Downcase(std::string& a)
	{
		uint32 size = (uint32) a.size();
		for (uint32 i=0; i<size; ++i)
			a[i] = tolower(a[i]);
	}

	//--------------------------------------------------------------------
	void Upcase(std::string& a)
	{
		uint32 size = (uint32) a.size();
		for (uint32 i=0; i<size; ++i)
			a[i] = toupper(a[i]);
	}

	//--------------------------------------------------------------------
	void ReplaceChar(std::string& a, char replace, char replaceWith)
	{
		for (uint32 i=0; i<a.size(); ++i)
		{
			if (a[i] == replace)
			{
				a[i] = replaceWith;
			}
		}

		/*
		uint32 i = a.find(replace);
		while (i != std::string::npos)
			a[i] = replaceWith;
		*/
	}
}
