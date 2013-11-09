#if !defined(DSC_STRING_H_)
#define DSC_STRING_H_

#include <string>

namespace dsc
{
	//data conversion functions
	const std::string ToString(double d, int nDigits = 8);
	const std::string ToString(float f, int nDigits = 8);
	const std::string ToString(long l, int radix = 10);
	const std::string ToString(unsigned long, int radix = 10);
	const std::string ToString(int i, int radix = 10);
	const std::string ToString(unsigned int ui, int radix = 10);
	const std::string ToString(short s, int radix = 10);
	const std::string ToString(unsigned short us, int radix = 10);
	const std::string ToString(bool b);

	//string utility fncs
	void Downcase(std::string& a);
	void Upcase(std::string& a);
	void ReplaceChar(std::string& a, char replace, char replaceWith);
}

#endif