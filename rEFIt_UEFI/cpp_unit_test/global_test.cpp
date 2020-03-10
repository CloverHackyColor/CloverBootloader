#include <Platform.h>
#include "../cpp_foundation/XStringW.h"

class XStringWTest : public XStringW
{
public:
	XStringWTest(const wchar_t *S) : XStringW()
	{
		StrCpy(S);
	}
};

XStringWTest global_str1(L"global_str1");
XStringWTest global_str2(L"global_str2");
